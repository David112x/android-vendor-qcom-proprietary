/*****************************************************************************
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/

#ifndef __QJPEGDMA_V4L2_ENGINE_H__
#define __QJPEGDMA_V4L2_ENGINE_H__

#include "cam_types.h"
#include "img_common.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "QImageDMAInterface.h"

/** QDma_ImgComp_t
 * @ptr: handle to imglib library
 * @img_core_get_comp: function pointer for img_core_get_comp
 * @core_ops: image core ops structure handle
 * @comp: component structure handle
**/
typedef struct {
  void *ptr;
  int (*img_core_get_comp) (img_comp_role_t role, char *name,
    img_core_ops_t *p_ops);
  img_core_ops_t core_ops;
  img_component_ops_t comp;
} QDmaImgComp_t;

/*===========================================================================
 * Class: QJPEGDMA_V4L2_Engine
 *
 * Description: This class represents the HW dma engine component
 *
 * Notes: none
 *==========================================================================*/
class QJPEGDMA_V4L2_Engine : public QImageDMAInterface, public QThreadObject
{
  public:

  /** ~QJPEGDMA_V4L2_Engine:
  *
  *  Destructor
  **/
  ~QJPEGDMA_V4L2_Engine();

  /** New:
   *  @aParams: dma engine parameters
   *
   *  2 phase contsructor
   **/
  static QJPEGDMA_V4L2_Engine* New(void *aPrivData_ptr,
    DmaEngineCallback aCallback);

  /** Execute:
   *
   *  thread execute function
   **/
  virtual void Execute();

  /** Stop:
  *
  *  stop the dma engine
  */
  virtual int Stop();

  /** Start:
  *
  *  starts the dma engine with crop enabled
  **/
  virtual int Start(QImage &aInImage, QImage &aOutImage,
    QICrop &aInCrop, bool sync = true);

  /** Start:
  *
  *  starts the dma engine
  **/
  virtual int Start(QImage &aInImage, QImage &aOutImage, bool sync = true);

  /** EventHandler
  *
  *  Event Handler for the hw dma engine component
  **/
  void EventHandler(img_frame_bundle_t *aFrameBundle_ptr,
    img_event_type aCallbackEvent);

  /** ReleaseSession
  *
  *  Method to relase session.
  **/
  void ReleaseSession();

  /** GetMaxDownScale:
   *
   *  Get max dma downscale factor
   **/
  float GetMaxDownScale();

  private:

  /** QJPEGDMA_V4L2_Engine:
  *
  *  Default Constructor
  **/
  QJPEGDMA_V4L2_Engine(void *aPrivData_ptr, DmaEngineCallback aCallback);

  /** getDmaYuvformat
   *
   *  translate image format to to cam format
   **/
  cam_format_t getDmaYuvformat(QIFormat fmt);

  /** ConfigureBuffers:
  *
  *  configure dma engine settings
  **/
  int Configure();

  /** Init:
   *
   *  loads and inits dma engine library
   **/
   bool Init();

  /** Load:
  *
  *  loads dma engine library
  **/
  int Load();

  /** UnLoad:
  *
  *  unloads dma engine library
  **/
  void UnLoad();

  /** StartDmaComp:
  *
  *  configure and start jpeg dma component
  **/
  int StartDmaComp();

  /** SendFramesToDmaComp:
  *
  *  Send frames to jpeg dma component
  **/
  int SendFramesToDmaComp();

  /** PopulateInFrameInfo:
  *
  *  populate input frame info
  **/
  int PopulateFrameInfo(img_frame_t &aFrame, QImage *aImage_ptr,
    img_dim_t &aDim);

  /** Transfer:
  *
  *  Start Transfer
  **/
  void Transfer();

private:

  /** mMutex
  *
  *  mutex object
  **/
  pthread_mutex_t mMutex;

  /** mLibMutex
  *
  *  mutex for library calls
  **/
  pthread_mutex_t mLibMutex;

  /** mCond
  *
  *  conditional object
  **/
  pthread_cond_t mCond;

  /** mCond
  *
  *  condition for library calls
  **/
  pthread_cond_t mLibCond;


  /** mTransferThread
  *
  *  Transfer thread
  **/
  QIThread mTransferThread;

  /** mInputImage
  *
  *  Input Image Object
  **/
  QImage *mInputImage_ptr;

  /** mOutputImage
  *
  *  Output Image Object
  **/
  QImage *mOutputImage_ptr;

  /** mInputCrop
  *
  *  Input crop Object
  **/
  QICrop mInputCrop;

  /** mState:
   *
   *  Current state
   **/
  QState mState;

  /** mDmaComp:
  *
  *  Handle to the JPEGDMA component
  **/
  QDmaImgComp_t mDmaComp;

  /** mOutputDone:
  *
  *  Flag to check if Output is done
  **/
  bool mOutputDone;


  /** mError:
   *
   *  last error value
   **/
  int mError;

  /** mDmaCompHandle:
  *
  *  Userspace Driver handle
  **/
  void *mDmaCompHandle_ptr;

  /** mInputDim
  *
  * Input image dimensions passed to userspace driver
  **/
  img_dim_t mInputDim;

  /** mOutputDim
  *
  * Input image dimensions passed to userspace driver
  **/
  img_dim_t mOutputDim;

  /** mCropRect
  *
  * Crop info passed to userspace driver
  **/
  img_rect_t mCropRect;

  /** mYuvFormat
  *
  * Yuv format passed to userspace driver
  **/
  cam_format_t mYuvFormat;

  /** mFramerate
  *
  * Framerate passed to userspace driver, used to calculate
  *   HW clock speed
  **/
  uint32_t mFramerate;

  /** mFrameId
  *
  * FrameId passed to userspace driver
  **/
  uint32_t mFrameId;

  /** mInputFrame
  *
  * Input frame passed to userspace driver
  **/
  img_frame_t mInputFrame;

  /** mOutputFrame
  *
  * Output frame passed to userspace driver
  **/
  img_frame_t mOutputFrame;

  /** mMeta
  *
  * Meta data passed to userspace driver
  **/
  img_meta_t mMeta;

  /** mStreaming
  *
  * Flag indicating streaming started
  **/
  bool mStreaming;

  /** mCallback
   *
   *  Callback for transfer completion
   **/
  DmaEngineCallback mCallback;

  /** mClientData
   *
   *  Client private data
   **/
  void *mClientData_ptr;

  /** mMaxDMADownScale
  *
  *   Max DMA downscale factor
  **/
  float mMaxDMADownScale;

};

#endif
