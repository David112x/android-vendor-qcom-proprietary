/*****************************************************************************
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *****************************************************************************/

#include "QJPEGDMAEngine.h"

/*============================================================================
 MACROS and CONSTANTS
 ============================================================================*/

#define TRANSFER_TIMEOUT 10000

/* plane count for semiplanar and planar*/
#define MONO_CNT 1
#define SP_CNT 2
#define P_CNT 3

#define FLOOR_PAD_TO_16(a)  ((a>>4)<<4)
#define MAX_JPEGDMA_OUT_SIZE  FLOOR_PAD_TO_16(0x4000000)

// Max DMA downscale factor
#define MAX_DMA_DOWNSCALE (16.0f)

#include <sched.h>

/*===========================================================================
 * Function: QJPEGDMAEngine
 *
 * Description: QJPEGDMAEngine constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QJPEGDMAEngine::QJPEGDMAEngine(void *aPrivData, DmaEngineCallback aCallback)
{
  QI_MUTEX_INIT(&mMutex);
  QI_COND_INIT(&mCond);
  mInputImage = NULL;
  mOutputImage = NULL;
  mState = ESTATE_IDLE;
  mDmaLibHandle = NULL;
  mIONInput = NULL;
  mIONOutput = NULL;
  mOutputDone = false;
  mError = QI_SUCCESS;
  mCallback = aCallback;
  mClientData = aPrivData;
  QI_MUTEX_INIT(&mLibMutex);
  QI_COND_INIT(&mLibCond);
  memset(&mDmaLib, 0x0, sizeof(QDmaLib_t));
  memset(&mTransferCfg, 0x0, sizeof(jpegdma_cmd_jpeg_transfer_cfg));
  memset(&mScaleCfg, 0, sizeof(jpegdma_cmd_scale_cfg));
  memset(&mInputCfg, 0, sizeof(jpegdma_cmd_input_cfg));

  for (int i = 0; i < QI_MAX_PLANES; i++) {
    mInAddr[i] = NULL;
    mOutAddr[i] = NULL;
  }
  /*enable synchronous mode*/
  mSyncMode = true;

  SetDynamicLogging("1");
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
void QJPEGDMAEngine::ReleaseSession()
{
  Stop();

  /*stop and join the dma engine thread*/
  mTransferThread.StopThread(this);

  if (mIONInput) {
    delete mIONInput;
    mIONInput = NULL;
  }

  if (mIONOutput) {
    delete mIONOutput;
    mIONOutput = NULL;
  }
}

/*===========================================================================
 * Function: ~QJPEGDMAEngine
 *
 * Description: QJPEGDMAEngine destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QJPEGDMAEngine::~QJPEGDMAEngine()
{
  Stop();

  int lrc = 0;

  if (mDmaLib.ptr) {
    mDmaLib.jpegdma_lib_release(mDmaLibHandle);
    if (lrc < 0)
      QIDBG_ERROR("%s:%d: release failed %d", __func__, __LINE__, lrc);
    mDmaLibHandle = NULL;
  }

  /*stop and join the dma engine thread*/
  mTransferThread.JoinThread();

  if (mIONInput) {
    delete mIONInput;
    mIONInput = NULL;
  }

  if (mIONOutput) {
    delete mIONOutput;
    mIONOutput = NULL;
  }

  QI_MUTEX_DESTROY(&mMutex);
  QI_COND_DESTROY(&mCond);

  /*unload the library*/
  UnLoad();
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
float QJPEGDMAEngine::GetMaxDownScale()
{
  return MAX_DMA_DOWNSCALE;
}

/*===========================================================================
 * Function: OutputHandler
 *
 * Description: Event handler for the dma engine
 *
 * Input parameters:
 *   aDmaLibHandle - Dma HW Obj
 *   aoutBuffer - Output Buffer
 *
 * Return values:
 *   true/false
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMAEngine::OutputHandler(jpegdma_hw_buf *aoutBuffer)
{
  if (NULL == aoutBuffer) {
    mOutputDone = false;
    mError = QI_ERR_GENERAL;
    return QI_SUCCESS;
  }

  QIDBG_MED("%s:%d] type %d", __func__, __LINE__, aoutBuffer->type);

  QI_LOCK(&mLibMutex);

  QIDBG_HIGH("%s %d:Frame done length %d", __func__, __LINE__,
      aoutBuffer->framedone_len);

  if (aoutBuffer->framedone_len > mOutputImage->Length()) {
    QIDBG_ERROR("%s %d:JPEG output length %d larger than buffer length %d",
        __func__, __LINE__, aoutBuffer->framedone_len, mOutputImage->Length());
    mError = QI_ERR_OUT_OF_BOUNDS;
    QI_UNLOCK(&mLibMutex);
    return QI_ERR_OUT_OF_BOUNDS;
  }
  if (mOutputImage->Fd() < 0) {
    //copy to output buffer if it not done in OMX layer
    memcpy(mOutputImage->BaseAddr(), aoutBuffer->vaddr,
        aoutBuffer->framedone_len);
  }

  mOutputImage->SetFilledLen(aoutBuffer->framedone_len);

  QI_UNLOCK(&mLibMutex);

  return QI_SUCCESS;
}

/*===========================================================================
 * Function: jpegdma_hw10_output_handler
 *
 * Description: Static function for output handler
 *
 * Input parameters:
 *   p_user - HW object pointer
 *   aBuffer - jpegdma_hw_buf Buffer
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int jpegdma_hw10_output_handler(void *p_user, jpegdma_hw_buf *aBuffer)
{
  QJPEGDMAEngine *lEngine = (QJPEGDMAEngine *) p_user;
  if (lEngine) {
    return lEngine->OutputHandler(aBuffer);
  }
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: InputHandler
 *
 * Description: Event handler for the dma engine
 *
 * Input parameters:
 *   aDmaLibHandle - Dma HW Obj
 *   aBuffer - jpegdma_hw_buf Buffer
 *
 * Return values:
 *   true/false
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMAEngine::InputHandler(jpegdma_hw_buf *aBuffer)
{
  QIDBG_MED("%s:%d] type %d", __func__, __LINE__, aBuffer->type);

  QI_LOCK(&mLibMutex);
  if (JPEGDMA_EVT_SESSION_DONE == aBuffer->type) {
    mOutputDone = true;
  } else if (JPEGDMA_EVT_ERROR == aBuffer->type) {
    mError = QI_ERR_GENERAL;
  }
  QI_SIGNAL(&mLibCond);
  QI_UNLOCK(&mLibMutex);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: jpegdma_hw10_input_handler
 *
 * Description: Static function for input handler
 *
 * Input parameters:
 *   p_user - HW object pointer
 *   aBuffer - jpegdma_hw_buf Buffer
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int jpegdma_hw10_input_handler(void *p_user, jpegdma_hw_buf *aBuffer)
{
  QJPEGDMAEngine *lEngine = (QJPEGDMAEngine *) p_user;
  if (lEngine) {
    return lEngine->InputHandler(aBuffer);
  }
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: EventHandler
 *
 * Description: Event handler for the dma engine
 *
 * Input parameters:
 *   aEvent - Event
 *
 * Return values:
 *   true/false
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMAEngine::EventHandler(int aEvent)
{
  QIDBG_MED("%s:%d] type %d", __func__, __LINE__, aEvent);

  QI_LOCK(&mLibMutex);
  if (JPEGDMA_EVT_SESSION_DONE == aEvent) {
    QIDBG_HIGH("%s %d: JPEG_EVT_SESSION_DONE", __func__, __LINE__);
    mOutputDone = true;
  } else if (JPEGDMA_EVT_ERROR == aEvent) {
    QIDBG_HIGH("%s %d: JPEG_EVT_ERROR", __func__, __LINE__);
    mError = QI_ERR_GENERAL;
  }
  QI_SIGNAL(&mLibCond);
  QI_UNLOCK(&mLibMutex);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: jpegdma_hw10_event_handler
 *
 * Description: Static function for event handler
 *
 * Input parameters:
 *   p_user - HW object pointer
 *   p_evt - pointer to the HW events
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int jpegdma_hw10_event_handler(void *p_user,
    jpegdma_hw_evt *aJpegCtrlCmd __unused,
    int aEvent)
{
  QJPEGDMAEngine *lEngine = (QJPEGDMAEngine *) p_user;
  if (lEngine) {
    return lEngine->EventHandler(aEvent);
  }
  return QI_SUCCESS;
}
/*===========================================================================
 * Function: ConfigureOutputBuffer
 *
 * Description: Configure the output buffer
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

int QJPEGDMAEngine::ConfigureOutputBuffer(int plane)
{
  uint32_t lBufferlength = 0, lOffset = 0;
  int lrc = QI_SUCCESS;
  QIPlane *lPlane = NULL;

  jpegdma_hw_buf lJpegBuf;
  memset(&lJpegBuf, 0x0, sizeof(jpegdma_hw_buf));

  //Configure the JPEG HW bufer structure
  QIDBG_MED("%s %d: Num of planes = %d", __func__, __LINE__,
      mOutputImage->PlaneCount());

  lJpegBuf.type = 0x0;

  int idx1 = plane_cnt_lut[mOutputImage->PlaneCount() - 1][plane].idx1;
  int idx2 = plane_cnt_lut[mOutputImage->PlaneCount() - 1][plane].idx2;

  if (idx1 == -1 || idx2 == -1) {
    QIDBG_ERROR("%s %d: Plance count %d plane idx %d not supported", __func__,
        __LINE__, mOutputImage->PlaneCount(), plane);
    return QI_ERR_INVALID_INPUT;
  }

  lPlane = mOutputImage->getPlane((QIPlane::Type) idx2);
  if (NULL == lPlane) {
    return QI_ERR_INVALID_INPUT;
  }

  lJpegBuf.y_len = mOutSize[idx1].Length();
  lJpegBuf.y_off = mOutputImage->getPlane((QIPlane::Type) idx2)->PhyOffset();

  QIDBG_MED("%s %d: Y-len = %d, y-offset =%d, cbcr_len = %d, cbcr_off = %d,"
      "cr_len = %d, cr_off = %d", __func__, __LINE__, lJpegBuf.y_len,
      lJpegBuf.y_off, lJpegBuf.cbcr_len, lJpegBuf.cbcr_off, lJpegBuf.cr_len,
      lJpegBuf.cr_offset);

  lPlane = mOutputImage->getPlane(QIPlane::PLANE_Y);
  if (NULL == lPlane) {
    return QI_ERR_INVALID_INPUT;
  }

  /*If the input ion buffer is not alread allocated, allocate it now*/
  if (lPlane->Fd() > 0) {
    lJpegBuf.vaddr = lPlane->Addr();
    lJpegBuf.fd = lPlane->Fd();
  } else {
    QIDBG_ERROR("%s %d: Only hw buffers are supported", __func__, __LINE__);
  }

  lrc = mDmaLib.jpegdma_lib_output_buf_enq(mDmaLibHandle, &lJpegBuf);
  return lrc;
}

/*===========================================================================
 * Function: populatePlaneAddr
 *
 * Description: Populate the plane addresses of the buffers
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
int QJPEGDMAEngine::populatePlaneAddr()
{
  QIPlane *lPlane = NULL;

  /*set the padded dimension*/
  if (MONO_CNT <= mInputImage->PlaneCount()) {
    lPlane = mInputImage->getPlane(QIPlane::PLANE_Y);
    if (NULL == lPlane) {
      QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    mInAddr[IY] = lPlane->ActualAddr();
    mInSize[IY] = lPlane->ActualSize();
    mInPadSize[IY] = lPlane->Size();
  }

  if (SP_CNT == mInputImage->PlaneCount()) {
    lPlane = mInputImage->getPlane(QIPlane::PLANE_CB_CR);
    if (NULL == lPlane) {
      QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    mInAddr[IC] = lPlane->ActualAddr();
    mInSize[IC] = lPlane->ActualSize();
    mInPadSize[IC] = lPlane->Size();
  } else if (P_CNT == mInputImage->PlaneCount()) {
    QIPlane::Type lType[2];

    if (QImage::IsCrCb(mInputImage->Format())) {
      lType[0] = QIPlane::PLANE_CR;
      lType[1] = QIPlane::PLANE_CB;
    } else {
      lType[0] = QIPlane::PLANE_CB;
      lType[1] = QIPlane::PLANE_CR;
    }

    for (int j = 0; j < 2; j++) {
      lPlane = mInputImage->getPlane(lType[j]);
      if (NULL == lPlane) {
        QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
        return QI_ERR_INVALID_INPUT;
      }
      mInAddr[j + 1] = lPlane->ActualAddr();
      mInSize[j + 1] = lPlane->ActualSize();
      mInPadSize[j + 1] = lPlane->Size();
    }
  }
  QIDBG_MED("%s:%d] plane address %p %p %p", __func__, __LINE__, mInAddr[0],
      mInAddr[1], mInAddr[2]);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: populatePlaneAddr
 *
 * Description: Populate the plane addresses of the buffers
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
int QJPEGDMAEngine::populateOutPlaneAddr()
{
  QIPlane *lPlane = NULL;

  /*set the padded dimension*/
  if (MONO_CNT <= mOutputImage->PlaneCount()) {
    lPlane = mOutputImage->getPlane(QIPlane::PLANE_Y);
    if (NULL == lPlane) {
      QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    mOutAddr[IY] = lPlane->ActualAddr();
    mOutSize[IY] = lPlane->ActualSize();
    mOutPadSize[IY] = lPlane->Size();
  }

  if (SP_CNT == mOutputImage->PlaneCount()) {
    lPlane = mOutputImage->getPlane(QIPlane::PLANE_CB_CR);
    if (NULL == lPlane) {
      QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    mOutAddr[IC] = lPlane->ActualAddr();
    mOutSize[IC] = lPlane->ActualSize();
    mOutPadSize[IC] = lPlane->Size();
  } else if (P_CNT == mOutputImage->PlaneCount()) {
    QIPlane::Type lType[2];

    if (QImage::IsCrCb(mOutputImage->Format())) {
      lType[0] = QIPlane::PLANE_CR;
      lType[1] = QIPlane::PLANE_CB;
    } else {
      lType[0] = QIPlane::PLANE_CB;
      lType[1] = QIPlane::PLANE_CR;
    }

    for (int j = 0; j < 2; j++) {
      lPlane = mOutputImage->getPlane(lType[j]);
      if (NULL == lPlane) {
        QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
        return QI_ERR_INVALID_INPUT;
      }
      mOutAddr[j + 1] = lPlane->ActualAddr();
      mOutSize[j + 1] = lPlane->ActualSize();
      mOutPadSize[j + 1] = lPlane->Size();
    }
  }
  QIDBG_MED("%s:%d] plane address %p %p %p", __func__, __LINE__, mOutAddr[0],
      mOutAddr[1], mOutAddr[2]);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: populateCrop
 *
 * Description: Populate crop parameters
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
int QJPEGDMAEngine::populateCrop()
{
  QIPlane *lPlane;
  QISubsampling lSubsample;
  uint32_t lX1, lY1, lX2, lY2;
  uint32_t semi_planar_mult;

  if (!mInputCrop.isValid()) {
    for (uint32_t i = 0; i < mInputImage->PlaneCount(); i++) {
      mInCrop[i].setCrop(0x0, 0x0, 0x0, 0x0);
    }
    return QI_SUCCESS;
  }
  lSubsample  = mInputImage->SubSampling();

  //Handle semi planar width
  switch (mInputImage->Format()) {
  case QI_YCRCB_SP:
  case QI_YCBCR_SP:
    semi_planar_mult = 2;
    break;
  default:
    semi_planar_mult = 1;
    break;
  }

  if (MONO_CNT <= mInputImage->PlaneCount()) {
    lPlane = mInputImage->getPlane(QIPlane::PLANE_Y);
    if (NULL == lPlane) {
      QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    mInCrop[IY] = mInputCrop;
  }

  if (SP_CNT == mInputImage->PlaneCount()) {
    lPlane = mInputImage->getPlane(QIPlane::PLANE_CB_CR);
    if (NULL == lPlane) {
      QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    lX1 = ceil(mInputCrop.Left() *
      semi_planar_mult * QImage::getChromaWidthFactor(lSubsample));
    lX2 = ceil(mInputCrop.Right() *
      semi_planar_mult * QImage::getChromaWidthFactor(lSubsample));
    lY1 = ceil(mInputCrop.Top() *
      QImage::getChromaHeightFactor(lSubsample));
    lY2 = ceil(mInputCrop.Bottom() *
      QImage::getChromaHeightFactor(lSubsample));
    mInCrop[IC].setCrop(lX1, lY1, lX2, lY2);
  } else if (P_CNT == mInputImage->PlaneCount()) {
    QIPlane::Type lType[2];

    if (QImage::IsCrCb(mInputImage->Format())) {
      lType[0] = QIPlane::PLANE_CR;
      lType[1] = QIPlane::PLANE_CB;
    } else {
      lType[0] = QIPlane::PLANE_CB;
      lType[1] = QIPlane::PLANE_CR;
    }

    for (int j = 0; j < 2; j++) {
      lPlane = mInputImage->getPlane(lType[j]);
      if (NULL == lPlane) {
        QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
        return QI_ERR_INVALID_INPUT;
      }
      if (j == 0) {
        lX1 = ceil(mInputCrop.Left() *
          semi_planar_mult * QImage::getChromaWidthFactor(lSubsample));
        lX2 = ceil(mInputCrop.Right() *
          semi_planar_mult * QImage::getChromaWidthFactor(lSubsample));
        lY1 = ceil(mInputCrop.Top() *
          QImage::getChromaHeightFactor(lSubsample));
        lY2 = ceil(mInputCrop.Bottom() *
          QImage::getChromaHeightFactor(lSubsample));
        mInCrop[j + 1].setCrop(lX1, lY1, lX2, lY2);
      } else {
        mInCrop[j + 1] = mInCrop[j];
      }
    }
  }

  return QI_SUCCESS;
}

/*===========================================================================
 * Function: ConfigureInputBuffer
 *
 * Description: Configure the input buffer
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

int QJPEGDMAEngine::ConfigureInputBuffer(int plane)
{
  uint32_t lBufferlength = 0, lOffset = 0;
  int lrc = QI_SUCCESS;
  QIPlane *lPlane = NULL;

  jpegdma_hw_buf lJpegBuf;
  memset(&lJpegBuf, 0x0, sizeof(jpegdma_hw_buf));

  //Configure the JPEG HW bufer structure
  QIDBG_MED("%s %d: Num of planes = %d", __func__, __LINE__,
      mInputImage->PlaneCount());

  lJpegBuf.type = 0x0;

  int idx1 = plane_cnt_lut[mInputImage->PlaneCount() - 1][plane].idx1;
  int idx2 = plane_cnt_lut[mInputImage->PlaneCount() - 1][plane].idx2;

  if (idx1 == -1 || idx2 == -1) {
    QIDBG_ERROR("%s %d: Plance count %d plane idx %d not supported", __func__,
        __LINE__, mInputImage->PlaneCount(), plane);
    return QI_ERR_INVALID_INPUT;
  }

  lPlane = mInputImage->getPlane((QIPlane::Type) idx2);
  if (NULL == lPlane) {
    QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return QI_ERR_INVALID_INPUT;
  }

  lJpegBuf.y_len = mInSize[idx1].Length();
  lJpegBuf.y_off = mInputImage->getPlane((QIPlane::Type) idx2)->PhyOffset();

  QIDBG_MED("%s %d: Y-len = %d, y-offset =%d, cbcr_len = %d, cbcr_off = %d,"
      "cr_len = %d, cr_off = %d", __func__, __LINE__, lJpegBuf.y_len,
      lJpegBuf.y_off, lJpegBuf.cbcr_len, lJpegBuf.cbcr_off, lJpegBuf.cr_len,
      lJpegBuf.cr_offset);

  lPlane = mInputImage->getPlane(QIPlane::PLANE_Y);
  if (NULL == lPlane) {
    QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return QI_ERR_INVALID_INPUT;
  }

  /*If the input ion buffer is not alread allocated, allocate it now*/
  if (lPlane->Fd() > 0) {
    lJpegBuf.vaddr = lPlane->Addr();
    lJpegBuf.fd = lPlane->Fd();
  } else {
    QIDBG_ERROR("%s %d: Only hw buffers are supported", __func__, __LINE__);
  }

  lrc = mDmaLib.jpegdma_lib_input_buf_enq(mDmaLibHandle, &lJpegBuf);
  return lrc;
}

/*===========================================================================
 * Function: getDmaPlaneType
 *
 * Description: translate ICL plane type to jpegdma lib plane type
 *
 * Input parameters:
 *   planeCnt - plane count of image
 *   planeIdx - plane index
 *
 * Return values:
 *   JPEGDMA_PLANE_TYPE_Y
 *   JPEGDMA_PLANE_TYPE_CB
 *   JPEGDMA_PLANE_TYPE_CR
 *   JPEGDMA_PLANE_TYPE_CBCR
 *
 * Notes: none
 *==========================================================================*/
uint32_t QJPEGDMAEngine::getDmaPlaneType(uint32_t planeCnt, uint32_t planeIdx)
{
  int plane = plane_cnt_lut[planeCnt - 1][planeIdx].idx2;
  uint32_t outPlane = 0xFF;
  switch (plane) {
  case QIPlane::PLANE_Y:
    outPlane = JPEGDMA_PLANE_TYPE_Y;
    break;
  case QIPlane::PLANE_CB:
    outPlane = JPEGDMA_PLANE_TYPE_CB;
    break;
  case QIPlane::PLANE_CR:
    outPlane = JPEGDMA_PLANE_TYPE_CR;
    break;
  case QIPlane::PLANE_CB_CR:
    outPlane = JPEGDMA_PLANE_TYPE_CBCR;
    break;
  default:
    QIDBG_ERROR("%s %d", __func__, __LINE__);
  }
  return outPlane;
}

/*===========================================================================
 * Function: ConfigureInputParams
 *
 * Description: Configure the unput params for the HW dma engine
 *
 * Input parameters:
 *  None
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMAEngine::ConfigureInputParams()
{
  int lrc = QI_SUCCESS;
  QISubsampling lSubsampling = mInputImage->SubSampling();
  QIFormat lFormat = mInputImage->Format();

  /* Set the Color Format */
  switch (lSubsampling) {
  case QI_H2V2: {
    mInputCfg[0].inputFormat = JPEGDMA_INPUT_H2V2;
    break;
  }
  case QI_H2V1: {
    mInputCfg[0].inputFormat = JPEGDMA_INPUT_H2V1;
    break;
  }
  case QI_H1V1: {
    mInputCfg[0].inputFormat = JPEGDMA_INPUT_H1V1;
    break;
  }
  case QI_H1V2: {
    mInputCfg[0].inputFormat = JPEGDMA_INPUT_H1V2;
    break;
  }
  }

  /* Set the CBCR ordering */
  switch (lFormat) {
  case QI_YCBCR_SP: {
    mInputCfg[0].input_cbcr_order = 0;
    break;
  }
  case QI_YCRCB_SP: {
    mInputCfg[0].input_cbcr_order = 1;
    break;
  }
  case QI_IYUV: {
    mInputCfg[0].input_cbcr_order = 0;
    break;
  }
  case QI_YUV2: {
    mInputCfg[0].input_cbcr_order = 1;
    break;
  }
  case QI_MONOCHROME: {
    mInputCfg[0].input_cbcr_order = 0;
    mInputCfg[0].inputFormat = JPEGDMA_INPUT_MONOCHROME;
    break;
  }
  default: {
    QIDBG_ERROR("%s %d: Format not supported = %d", __func__, __LINE__,
        lFormat);
    return QI_ERR_INVALID_INPUT;
  }

  } /*end of switch*/

  for (uint32_t i = 0; i < mInputImage->PlaneCount(); i++) {
    mInputCfg[i].input_cbcr_order = mInputCfg[0].input_cbcr_order;
    mInputCfg[i].inputFormat = mInputCfg[0].inputFormat;
    mInputCfg[i].image_height = mInSize[i].Height();
    mInputCfg[i].image_width = mInSize[i].Width();
    mInputCfg[i].stride = mInPadSize[i].Width();
    mInputCfg[i].scanline = mInPadSize[i].Height();
    mInputCfg[i].plane_type = getDmaPlaneType(mInputImage->PlaneCount(), i);
    QIDBG_MED("%s %d: mInputConfig[%u]:Width = %d, Height = %d,"
        "cbcr order = %d, format = %d,"
        "num_of_input_plns = %d", __func__, __LINE__, i,
        mInputCfg[i].image_width, mInputCfg[i].image_height,
        mInputCfg[i].input_cbcr_order, mInputCfg[i].inputFormat,
        mInputImage->PlaneCount());
  }

  return lrc;

}
/*===========================================================================
 * Function: ConfigureScaleParams
 *
 * Description: Configure the HW decoder component
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMAEngine::ConfigureScaleParams()
{
  int lrc = QI_SUCCESS;
  QISize lOutSize = mOutputImage->ActualSize();

  for (int i = 0; i < (int) mInputImage->PlaneCount(); i++) {

    if (mInCrop[i].isValid()) {
      mScaleCfg[i].h_offset = mInCrop[i].Left();
      mScaleCfg[i].scale_input_width = mInCrop[i].Width();
      mScaleCfg[i].v_offset = mInCrop[i].Top();
      mScaleCfg[i].scale_input_height = mInCrop[i].Height();
      mScaleCfg[i].crop_enable = true;
    } else {
      mScaleCfg[i].h_offset = 0;
      mScaleCfg[i].scale_input_width = mInSize[i].Width();
      mScaleCfg[i].v_offset = 0;
      mScaleCfg[i].scale_input_height = mInSize[i].Height();
      mScaleCfg[i].crop_enable = false;
    }

    mScaleCfg[i].output_width = mOutSize[i].Width();
    mScaleCfg[i].output_height =  mOutSize[i].Height();
    mScaleCfg[i].output_stride = mOutPadSize[i].Width();
    mScaleCfg[i].output_scanline = mOutPadSize[i].Height();

    mScaleCfg[i].scale_enable = false;
    if (mScaleCfg[i].scale_input_width != mScaleCfg[i].output_width ||
       mScaleCfg[i].scale_input_height != mScaleCfg[i].output_height) {
       mScaleCfg[i].scale_enable = true;
    }

  }
  return lrc;
}

/*===========================================================================
 * Function: prepareHWConfig
 *
 * Description: Prepare a configuration without setting to the lib.
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
int QJPEGDMAEngine::prepareHWConfig()
{
  int lrc = QI_SUCCESS;

  lrc = ConfigureInputParams();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring Scale params", __func__, __LINE__);
    return lrc;
  }

  lrc = ConfigureScaleParams();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring Scale params", __func__, __LINE__);
    return lrc;
  }

  mTransferCfg.speed_mode = JPEGDMA_SPEED_HIGH;

  QIDBG_MED("%s %d: mScaleCfg = %p", __func__, __LINE__, &mScaleCfg);

  return lrc;
}

/*===========================================================================
 * Function: SetHWConfig
 *
 * Description: Configure the HW dma engine component
 *
 * Input parameters:
 *   plane - plane number to configure
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMAEngine::SetHWConfig(int plane)
{
  int lrc = QI_SUCCESS;

  QIDBG_MED("%s %d: Plane %d Scale enabled = %d, crop enabled = %d, "
      "crop width = %d, crop height = %d, output width = %d out height = %d",
      __func__, __LINE__, plane, mScaleCfg[plane].scale_enable,
      mScaleCfg[plane].crop_enable, mScaleCfg[plane].scale_input_width,
      mScaleCfg[plane].scale_input_height, mScaleCfg[plane].output_width,
      mScaleCfg[plane].output_height);

  QIDBG_MED("%s %d: mScaleCfg = %p", __func__, __LINE__, &mScaleCfg);
  lrc = mDmaLib.jpegdma_lib_hw_config(mDmaLibHandle, &mInputCfg[plane],
      &mTransferCfg, &mScaleCfg[plane]);

  return lrc;
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
int QJPEGDMAEngine::Configure()
{
  int lrc = QI_SUCCESS;

  lrc = populatePlaneAddr();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring plane address", __func__, __LINE__);
    return lrc;
  }

  lrc = populateCrop();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring crop", __func__, __LINE__);
    return lrc;
  }

  lrc = populateOutPlaneAddr();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error configuring output plane address", __func__,
      __LINE__);
    return lrc;
  }
  lrc = prepareHWConfig();
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error preparing HW config", __func__, __LINE__);
    return lrc;
  }

  return lrc;
}
/*===========================================================================
 * Function: Start
 *
 * Description: Started the hardware dma engine
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
int QJPEGDMAEngine::Start(QImage &aInImage, QImage &aOutImage, bool sync)
{
  int lrc = QI_SUCCESS;

  QIDBG_MED("%s:%d] E state %d", __func__, __LINE__, mState);
  QI_LOCK(&mMutex);
  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_INVALID_OPERATION;
  }

  mInputImage = &aInImage;
  mOutputImage = &aOutImage;

  if ((NULL == mInputImage) || (NULL == mOutputImage)) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_INVALID_OPERATION;
  }
  mInputCrop.setCrop(0x0, 0x0, 0x0, 0x0);

  lrc = Configure();
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] config failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_GENERAL;
  }

  mState = ESTATE_ACTIVE;
  if (sync == false) {
    lrc = mTransferThread.StartThread(this);
    if (lrc < 0) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      QI_UNLOCK(&mMutex);
      return lrc;
    }
  } else {
    QI_UNLOCK(&mMutex);
    Transfer();
    return lrc;
  }

  QI_UNLOCK(&mMutex);
  return lrc;
}

/*===========================================================================
 * Function: Start
 *
 * Description: Started the hardware dma engine with crop enabled
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
int QJPEGDMAEngine::Start(QImage &aInImage, QImage &aOutImage,
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

  mInputImage = &aInImage;
  mOutputImage = &aOutImage;

  if ((NULL == mInputImage) || (NULL == mOutputImage)) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_INVALID_OPERATION;
  }
  mInputCrop = aInCrop;

  lrc = Configure();
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] config failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_GENERAL;
  }

  mState = ESTATE_ACTIVE;
  if (sync == false) {
    lrc = mTransferThread.StartThread(this);
    if (lrc < 0) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
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
  return lrc;
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
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMAEngine::Stop()
{
  int lrc = QI_SUCCESS;

  QIDBG_MED("%s:%d] mState %d", __func__, __LINE__, mState);

  QI_LOCK(&(mMutex));

  if (mState != ESTATE_ACTIVE) {
    QI_UNLOCK(&(mMutex));
    return QI_SUCCESS;
  }

  mState = ESTATE_STOP_REQUESTED;

  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  QI_WAIT(&(mCond), &(mMutex));
  QIDBG_MED("%s:%d] ", __func__, __LINE__);

  QI_UNLOCK(&(mMutex));

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
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
void QJPEGDMAEngine::Execute()
{
  if (mTransferThread.IsSelf()) {
    /* dma engine thread */
    Transfer();
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
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
void QJPEGDMAEngine::Transfer()
{
  int lrc = QI_SUCCESS;
  struct jpegdma_hw_evt lEvent;
  struct jpegdma_hw_buf lInputBuf;
  struct jpegdma_hw_buf lOutputBuf;

  QIDBG_MED("%s:%d] E", __func__, __LINE__);
  for (int i = 0; i < (int) mInputImage->PlaneCount(); i++) {
    lrc = SetHWConfig(i);
    if (lrc != QI_SUCCESS) {
      QIDBG_ERROR("%s %d: Error configuring HW params", __func__, __LINE__);
      goto transfer_error;
    }
    lrc = ConfigureOutputBuffer(i);
    if (lrc != QI_SUCCESS) {
      QIDBG_ERROR("%s %d: Error configuring output buffer", __func__,
          __LINE__);
      goto transfer_error;
    }
    lrc = ConfigureInputBuffer(i);
    if (lrc != QI_SUCCESS) {
      QIDBG_ERROR("%s %d: Error configuring input buffer", __func__, __LINE__);
      goto transfer_error;
    }
    lrc = mDmaLib.jpegdma_lib_transfer(mDmaLibHandle);
    if (QI_ERROR(lrc)) {
      QIDBG_ERROR("%s:%d] Error Encoding ", __func__, __LINE__);
      goto transfer_error;
    }

    if (false == mSyncMode) {
      QIDBG_MED("%s:%d] waiting for session done event ", __func__, __LINE__);

      QI_LOCK(&mLibMutex);
      if (!mOutputDone && QI_SUCCEEDED(mError)) {
        lrc = QIThread::WaitForCompletion(&mLibCond, &mLibMutex,
            TRANSFER_TIMEOUT);
      }
      QI_UNLOCK(&mLibMutex);

      QIDBG_MED("%s:%d] wait done %d", __func__, __LINE__, lrc);

      if (QI_ERROR(mError) || (QI_ERR_TIMEOUT == lrc)) {
        QIDBG_ERROR("%s:%d] error event", __func__, __LINE__);
        goto transfer_error;
      }

      lrc = mDmaLib.jpegdma_lib_wait_done(mDmaLibHandle);
      if (lrc < 0) {
        QIDBG_ERROR("%s:%d: jpegdma_lib_wait_done failed", __func__, __LINE__);
        goto transfer_error;
      }
    } else {
      lrc = mDmaLib.jpegdma_lib_get_event(mDmaLibHandle, &lEvent);
      if (QI_ERROR(lrc)) {
        QIDBG_ERROR("%s:%d] Error cannot get event", __func__, __LINE__);
        goto transfer_error;
      }


      lrc = mDmaLib.jpegdma_lib_get_input(mDmaLibHandle, &lInputBuf);
      if (QI_ERROR(lrc)) {
        QIDBG_ERROR("%s:%d] Error cannot get input", __func__, __LINE__);
        goto transfer_error;
      }


      lrc = mDmaLib.jpegdma_lib_get_output(mDmaLibHandle, &lOutputBuf);
      if (QI_ERROR(lrc)) {
        QIDBG_ERROR("%s:%d] Error cannot get input", __func__, __LINE__);
        goto transfer_error;
      }

      QIDBG_MED("%s:%d] DMA complete for plane [%d] error %d",
        __func__, __LINE__, i, mError);
      if (QI_ERROR(mError)) {
        goto transfer_error;
      }
    }
  }

  QIDBG_MED("%s:%d] DMA complete for all planes state %d",
    __func__, __LINE__, mState);
  QI_LOCK(&mMutex);
  if (mState == ESTATE_STOP_REQUESTED) {
    goto stop;
  }
  mState = ESTATE_IDLE;
  QI_UNLOCK(&mMutex);

  if (mCallback) {
    QIDBG_MED("%s:%d] Send client callback", __func__, __LINE__);
    mCallback(mClientData, mOutputImage , QI_SUCCESS);
  }

  return;

stop:
  mState = ESTATE_IDLE;
  QI_SIGNAL(&mCond);
  QI_UNLOCK(&mMutex);
  return;

transfer_error:

  QI_LOCK(&mMutex);
  if (mState != ESTATE_STOP_REQUESTED) {
    lrc = mDmaLib.jpegdma_lib_stop(mDmaLibHandle);
    if (lrc < 0) {
      QIDBG_ERROR("%s:%d: Failed to stop the HW dma engine", __func__,
          __LINE__);
    }
  } else {
    QI_SIGNAL(&mCond);
  }
  mState = ESTATE_IDLE;
  QI_UNLOCK(&mMutex);

  if (mCallback) {
    mCallback(mClientData, mOutputImage, QI_ERR_GENERAL);
  }

  return;
}

/*===========================================================================
 * Function: Load
 *
 * Description: Loads the userspace dma engine driver
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
int QJPEGDMAEngine::Load()
{
  if (mDmaLib.ptr) {
    QIDBG_ERROR("%s:%d: library already loaded", __func__, __LINE__);
    return QI_ERR_NOT_FOUND;
  }
  mDmaLib.ptr = dlopen("libjpegdmahw.so", RTLD_NOW);
  if (!mDmaLib.ptr) {
    QIDBG_ERROR("%s:%d] Error opening JpegE library", __func__, __LINE__);
    return QI_ERR_NOT_FOUND;
  }

  *(void **) &(mDmaLib.jpegdma_lib_init) = dlsym(mDmaLib.ptr,
      "jpegdma_lib_init");
  if (!mDmaLib.jpegdma_lib_init) {
    QIDBG_ERROR("%s:%d] Error linking jpegdma_lib_init", __func__, __LINE__);
    dlclose(mDmaLib.ptr);
    mDmaLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **) &(mDmaLib.jpegdma_lib_transfer) = dlsym(mDmaLib.ptr,
      "jpegdma_lib_transfer");
  if (!mDmaLib.jpegdma_lib_transfer) {
    QIDBG_ERROR("%s:%d] Error linking jpegdma_lib_transfer", __func__,
        __LINE__);
    dlclose(mDmaLib.ptr);
    mDmaLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **) &(mDmaLib.jpegdma_lib_hw_config) = dlsym(mDmaLib.ptr,
      "jpegdma_lib_hw_config");
  if (!mDmaLib.jpegdma_lib_hw_config) {
    QIDBG_ERROR("%s:%d] Error linking jpegdma_lib_hw_config", __func__,
        __LINE__);
    dlclose(mDmaLib.ptr);
    mDmaLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **) &(mDmaLib.jpegdma_lib_input_buf_enq) = dlsym(mDmaLib.ptr,
      "jpegdma_lib_input_buf_enq");
  if (!mDmaLib.jpegdma_lib_input_buf_enq) {
    QIDBG_ERROR("%s:%d] Error linking jpegdma_lib_input_buf_enq", __func__,
        __LINE__);
    dlclose(mDmaLib.ptr);
    mDmaLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **) &(mDmaLib.jpegdma_lib_output_buf_enq) = dlsym(mDmaLib.ptr,
      "jpegdma_lib_output_buf_enq");
  if (!mDmaLib.jpegdma_lib_output_buf_enq) {
    QIDBG_ERROR("%s:%d] Error linking jpegdma_lib_output_buf_enq", __func__,
        __LINE__);
    dlclose(mDmaLib.ptr);
    mDmaLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **) &(mDmaLib.jpegdma_lib_release) = dlsym(mDmaLib.ptr,
      "jpegdma_lib_release");
  if (!mDmaLib.jpegdma_lib_release) {
    QIDBG_ERROR("%s:%d] Error linking jpegdma_lib_release", __func__,
      __LINE__);
    dlclose(mDmaLib.ptr);
    mDmaLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **) &(mDmaLib.jpegdma_lib_wait_done) = dlsym(mDmaLib.ptr,
      "jpegdma_lib_wait_done");
  if (!mDmaLib.jpegdma_lib_wait_done) {
    QIDBG_ERROR("%s:%d] Error linking jpegdma_lib_wait_done", __func__,
        __LINE__);
    dlclose(mDmaLib.ptr);
    mDmaLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **) &(mDmaLib.jpegdma_lib_stop) = dlsym(mDmaLib.ptr,
      "jpegdma_lib_stop");
  if (!mDmaLib.jpegdma_lib_stop) {
    QIDBG_ERROR("%s:%d] Error linking jpegdma_lib_stop", __func__, __LINE__);
    dlclose(mDmaLib.ptr);
    mDmaLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **) &(mDmaLib.jpegdma_lib_get_event) = dlsym(mDmaLib.ptr,
      "jpegdma_lib_get_event");
  if (!mDmaLib.jpegdma_lib_get_event) {
    QIDBG_ERROR("%s:%d] Error linking jpegdma_lib_get_event", __func__,
        __LINE__);
    dlclose(mDmaLib.ptr);
    mDmaLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **) &(mDmaLib.jpegdma_lib_get_input) = dlsym(mDmaLib.ptr,
      "jpegdma_lib_get_input");
  if (!mDmaLib.jpegdma_lib_get_input) {
    QIDBG_ERROR("%s:%d] Error linking jpegdma_lib_get_input", __func__,
        __LINE__);
    dlclose(mDmaLib.ptr);
    mDmaLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }

  *(void **) &(mDmaLib.jpegdma_lib_get_output) = dlsym(mDmaLib.ptr,
      "jpegdma_lib_get_output");
  if (!mDmaLib.jpegdma_lib_get_output) {
    QIDBG_ERROR("%s:%d] Error linking jpegdma_lib_get_output", __func__,
        __LINE__);
    dlclose(mDmaLib.ptr);
    mDmaLib.ptr = NULL;
    return QI_ERR_NOT_FOUND;
  }
  QIDBG_MED("%s:%d] JpegDMA library loaded successfully", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: UnLoad
 *
 * Description: UnLoads the dma engine userspace driver
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QJPEGDMAEngine::UnLoad()
{
  int rc = 0;
  QIDBG_MED("%s:%d: ptr %p", __func__, __LINE__, mDmaLib.ptr);
  if (mDmaLib.ptr) {
    rc = dlclose(mDmaLib.ptr);
    if (rc < 0)
      QIDBG_ERROR("%s:%d] error %s", __func__, __LINE__, dlerror());
    mDmaLib.ptr = NULL;
  }
}

/*===========================================================================
 * Function: New
 *
 * Description: 2 phase constructor for QImageHW10Decoder
 *
 * Input parameters:
 *   aParams - decoder params
 *
 * Return values:
 *   decoder interface pointer
 *
 * Notes: none
 *==========================================================================*/
QJPEGDMAEngine* QJPEGDMAEngine::New(void *aPrivData,
    DmaEngineCallback aCallback)
{
  QJPEGDMAEngine* lDma = new QJPEGDMAEngine(aPrivData, aCallback);
  if (NULL == lDma) {
    QIDBG_ERROR("%s:%d] no memory", __func__, __LINE__);
    return NULL;
  }
  if (!lDma->Init()) {
    QIDBG_ERROR("%s:%d] validation failed", __func__, __LINE__);
    delete lDma;
    return NULL;
  }
  return lDma;
}

/*===========================================================================
 * Function: Init
 *
 * Description: Function to check if transfer params are valid
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   true/false
 *
 * Notes: none
 *==========================================================================*/
bool QJPEGDMAEngine::Init()
{
  int lrc = QI_SUCCESS;
  int lDmaHwFd = -1;

  QI_LOCK(&mMutex);


  lrc = Load();
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d: cannot load library", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return false;
  }

  if (false == mSyncMode) {
    lDmaHwFd = mDmaLib.jpegdma_lib_init(&mDmaLibHandle, this,
        jpegdma_hw10_event_handler, jpegdma_hw10_output_handler,
        jpegdma_hw10_input_handler);
  } else {
    lDmaHwFd = mDmaLib.jpegdma_lib_init(&mDmaLibHandle, this, NULL, NULL,
        NULL);
  }
  if (lDmaHwFd < 0 || !mDmaLibHandle) {
    QIDBG_ERROR("%s %d: Failed to Initialize the Dma HW lib", __func__,
        __LINE__);
    UnLoad();
    QI_UNLOCK(&mMutex);
    return false;
  }
  QI_UNLOCK(&mMutex);
  return true;
}

const QJPEGDMAEngine::plane_idx_t QJPEGDMAEngine::plane_cnt_lut[][3] = {
    { { IY, QIPlane::PLANE_Y }, { -1, -1 }, { -1, -1 } },
    { { IY, QIPlane::PLANE_Y }, { IC, QIPlane::PLANE_CB_CR }, { -1, -1 } },
    { { IY, QIPlane::PLANE_Y }, { IC1, QIPlane::PLANE_CB },
    { IC2, QIPlane::PLANE_CR } }
};
