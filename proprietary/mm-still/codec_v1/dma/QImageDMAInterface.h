/*****************************************************************************
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/

#ifndef __QIMAGE_DMA_INTERFACE_H__
#define __QIMAGE_DMA_INTERFACE_H__

#include "QImage.h"
#include "QICrop.h"
#include "QIThread.h"
#include <dlfcn.h>

/** DmaEngineCallback
 *   @privData: private data
 *   @readyImage: ready image
 *   @error: error
 *
 *   DMA engine call back function
 **/
typedef void (*DmaEngineCallback)(void *privData, QImage *readyImage,
  int error);

/*===========================================================================
 * Class: QImageDMAInterface
 *
 * Description: This class represents the interface for all core DMA
 *   classes.
 *
 * Notes: none
 *==========================================================================*/
class QImageDMAInterface {

public:

  /** QState:
   *  ESTATE_IDLE: Idle state
   *  ESTATE_ACTIVE: Active state
   *  ESTATE_STOP_REQUESTED: Stop pending state
   *  ESTATE_STOPPED: Stopped state
   *
   *  State of dma engine
   **/
  typedef enum {
    ESTATE_IDLE,
    ESTATE_ACTIVE,
    ESTATE_STOP_REQUESTED,
    ESTATE_STOPPED,
  } QState;

  /** QImageDMAInterface:
   *
   *  constructor
   **/
  QImageDMAInterface() {}

  /** ~QImageDMAInterface:
   *
   *  virtual destructor
   **/
  virtual ~QImageDMAInterface() {}

  /** Start:
   *  @aInImage: Input image
   *  @aOutImage: Output image
   *  @aInCrop: Crop dimension
   *  @sync: flag for sync mode
   *
   *  starts the DMA
   **/
  virtual int Start(QImage &aInImage, QImage &aOutImage,
    QICrop &aInCrop, bool sync = true) = 0;

  /** Start:
   *  @aInImage: Input image
   *  @aOutImage: Output image
   *  @sync: flag for sync mode
   *
   *  starts the dma engine
   **/
  virtual int Start(QImage &aInImage, QImage &aOutImage,
    bool sync = true) = 0;

  /** Stop:
   *
   *  stops the encoder
   **/
  virtual int Stop() = 0;

  /** ReleaseSession:
   *
   *  release the current encoding session
   **/
  virtual void ReleaseSession() = 0;

  /** GetMaxDownScale:
   *
   *  Get max dma downscale factor
   **/
  virtual float GetMaxDownScale() = 0;

};

#endif //__QIMAGE_DMA_INTERFACE_H__
