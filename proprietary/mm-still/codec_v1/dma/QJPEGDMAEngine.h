/*****************************************************************************
* Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/

#ifndef __QJPEGDMAENGINE_H__
#define __QJPEGDMAENGINE_H__

#include "QIDbg.h"

#include "QIONBuffer.h"
#include "jpegdma_lib.h"
#include "jpegdma_lib_common.h"
#include "QImageDMAInterface.h"

#define IY 0
#define IC 1
#define IC1 1
#define IC2 2

/** QDmaLib_t
 *  @jpegdma_lib_init: initializes the userspace driver
 *  @jpegdma_lib_release: deinitializes the userspace driver
 *  @jpegdma_lib_transfer: start encoding
 *  @jpegdma_lib_input_buf_enq: input buffer configuration
 *  @jpegdma_lib_output_buf_enq: output buffer configuration
 *  @jpegdma_lib_wait_done: wait for threads to complete
 *  @jpegdma_lib_hw_config: configure the dma engine registers
 *  @jpegdma_lib_stop: stop encoding
 *  @jpegdma_lib_get_event: get event (blocking call)
 *  @jpegdma_lib_get_input: get input buffer
 *  @jpegdma_lib_get_output: get output buffer
 *  @ptr : library handle
 *
 *  Jpeg 1.0 dma engine userspace driver function pointers
 **/
typedef struct {

 int (*jpegdma_lib_init) (jpegdma_hw_obj_t * jpegdma_hw_obj, void *p_userdata,
    int (*event_handler) (jpegdma_hw_obj_t,struct jpegdma_hw_evt *,int event),
    int (*output_handler) (jpegdma_hw_obj_t, struct jpegdma_hw_buf *),
    int (*input_handler) (jpegdma_hw_obj_t, struct jpegdma_hw_buf *));

  int (*jpegdma_lib_release)(jpegdma_hw_obj_t jpegdma_hw_obj);

  int (*jpegdma_lib_hw_config) (jpegdma_hw_obj_t jpegdma_hw_obj,
    jpegdma_cmd_input_cfg * p_input_cfg,
    jpegdma_cmd_jpeg_transfer_cfg * p_transfer_cfg,
    jpegdma_cmd_scale_cfg * p_scale_cfg);

  int (*jpegdma_lib_input_buf_enq) (jpegdma_hw_obj_t jpegdma_hw_obj,
    struct jpegdma_hw_buf *);

  int (*jpegdma_lib_output_buf_enq) (jpegdma_hw_obj_t jpegdma_hw_obj,
    struct jpegdma_hw_buf *);

  int (*jpegdma_lib_transfer) (jpegdma_hw_obj_t jpegdma_hw_obj);

  int (*jpegdma_lib_wait_done) (jpegdma_hw_obj_t jpegdma_hw_obj);

  int (*jpegdma_lib_stop) (jpegdma_hw_obj_t jpegdma_hw_obj);

  int (*jpegdma_lib_get_event)(jpegdma_hw_obj_t jpegdma_hw_obj,
    struct jpegdma_hw_evt *);

  int (*jpegdma_lib_get_input)(jpegdma_hw_obj_t jpegdma_hw_obj,
    struct jpegdma_hw_buf *);

  int (*jpegdma_lib_get_output)(jpegdma_hw_obj_t jpegdma_hw_obj,
    struct jpegdma_hw_buf *);

  void *ptr;

} QDmaLib_t;

/*===========================================================================
 * Class: QJPEGDMAEngine
 *
 * Description: This class represents the HW dma engine component
 *
 * Notes: none
 *==========================================================================*/
class QJPEGDMAEngine : public QImageDMAInterface, public QThreadObject
{
  public:

  typedef struct {
    int idx1;
    int idx2;
  } plane_idx_t;

  /** ~QJPEGDMAEngine:
  *
  *  Destructor
  **/
  ~QJPEGDMAEngine();

  /** New:
   *  @aParams: dma engine parameters
   *
   *  2 phase contsructor
   **/
  static QJPEGDMAEngine* New(void *privData, DmaEngineCallback aCallback);

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
  *  starts the dma engine
  **/
  virtual int Start(QImage &aInImage, QImage &aOutImage, bool sync = true);

  /** Start:
  *
  *  starts the dma engine with crop enabled
  **/
  virtual int Start(QImage &aInImage, QImage &aOutImage,
    QICrop &aInCrop, bool sync = true);

    /** OutputHandler
  *
  *  Output Handler for the hw dma engine component
  **/
  int OutputHandler(jpegdma_hw_buf *aoutBuffer);

  /** InputHandler
  *
  *  Input Handler for the hw dma engine component
  **/
  int InputHandler(jpegdma_hw_buf *aBuffer);

  /** EventHandler
  *
  *  Event Handler for the hw dma engine component
  **/
  int EventHandler(int aEvent);

  /** ReleaseSession
  *
  *  Method to relase session, takes a flag as input to stop
  *  thread or not
  **/
  void ReleaseSession();

  /** GetMaxDownScale:
   *
   *  Get max dma downscale factor
   **/
  float GetMaxDownScale();

  private:
  static const plane_idx_t plane_cnt_lut[][3];
  /** QJPEGDMAEngine:
  *
  *  Default Constructor
  **/
  QJPEGDMAEngine(void *privData, DmaEngineCallback aCallback);

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

  /** ConfigureOutputBuffer:
  *
  *  configure output buffers
  **/
  int ConfigureOutputBuffer(int plane);

  /** ConfigureOutputBuffer
  *
  *  configure output buffers
  **/
  int ConfigureInputBuffer(int plane);

  /** ConfigureScaleParams
  *
  *  Configure scale parameters
  **/
  int ConfigureScaleParams();

  /** ConfigureInputParams
  *
  *  Configure Input parameters
  **/
  int ConfigureInputParams();

  /** ConfigureTables
  *
  *  Configure Input parameters
  **/
  int ConfigureTables();

  /** populatePlaneAddr:
  *
  *  populate plane address
  **/
  int populatePlaneAddr();

  /** populateOutPlaneAddr:
   *
   *  populate plane address
   **/
  int populateOutPlaneAddr();

  /** populateCrop:
   *
   *  populate crop
   **/
  int populateCrop();

  /** SetHWConfig:
   *
   *  prepare transfer configuration for all planes
   **/
  int prepareHWConfig();

  /** SetHWConfig:
  *
  *  Configure HW specific data
  *  @plane: plane number to configure
  **/
  int SetHWConfig(int plane);

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
  QImage *mInputImage;

  /** mOutputImage
  *
  *  Output Image Object
  **/
  QImage *mOutputImage;

  /** mInputCrop
  *
  *  Input crop Object
  **/
  QICrop mInputCrop;

  /** mIONInput
  *
  *  Input Ion buffer
  **/
  QIONBuffer *mIONInput;

  /** mIONOutput
  *
  *  Output Ion buffer
  **/
  QIONBuffer *mIONOutput;

  /** mInAddr
  *
  *  Array of input addresses
  **/
  uint8_t *mInAddr[QI_MAX_PLANES];

  /** mOutAddr
   *
   *  Array of input addresses
   **/
  uint8_t *mOutAddr[QI_MAX_PLANES];

  /** mOutSize
  *
  *  Array of input sizes
  **/
  QISize mInSize[QI_MAX_PLANES];

  /** mInCrop
  *
  *  Array of input crop objects
  **/
  QICrop mInCrop[QI_MAX_PLANES];

  /** mOutSize
   *
   *  Array of input sizes
   **/
  QISize mOutSize[QI_MAX_PLANES];

  /** mState:
   *
   *  Current state
   **/
  QState mState;

  /** mDmaLib:
  *
  *  Handle to the JPEGE Library
  **/
  QDmaLib_t mDmaLib;

  /** mDmaLibHandle:
  *
  *  Userspace Driver handle
  **/
  void *mDmaLibHandle;

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

  /** mTransferCfg
  *
  * Transfer data(tables) for the HW dma engine
  **/
  jpegdma_cmd_jpeg_transfer_cfg mTransferCfg;

  /** mScaleCfg
  *
  * Scale data for the HW dma engine
  **/
  jpegdma_cmd_scale_cfg mScaleCfg[QI_MAX_PLANES];

  /** mInputCfg
  *
  * Input data for the HW dma engine
  **/
  jpegdma_cmd_input_cfg mInputCfg[QI_MAX_PLANES];

  /** mSyncMode
  *
  * Use synchronous mode for userspace driver interactions
  **/
  bool mSyncMode;

  /** mInPadSize
  *
  *  Array of input padded sizes
  **/
  QISize mInPadSize[QI_MAX_PLANES];

  /** mOutPadSize
   *
   *  Array of input padded sizes
   **/
  QISize mOutPadSize[QI_MAX_PLANES];

  /** mCallback
   *
   *  Callback for transfer completion
   **/
  DmaEngineCallback mCallback;

  /** mClientData
   *
   *  Client private data
   **/
  void *mClientData;

  /** getDmaPlaneType
   *
   *  translate plane type to dma lib type
   **/
  uint32_t getDmaPlaneType(uint32_t planeCnt, uint32_t planeIdx);

};

#endif
