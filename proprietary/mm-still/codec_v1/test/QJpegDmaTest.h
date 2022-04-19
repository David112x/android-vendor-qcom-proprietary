/*****************************************************************************
* Copyright (c) 2012-2015 Qualcomm Technologies, Inc.  All Rights Reserved.  *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QJPEGDMA_TEST_H__
#define __QJPEGDMA_TEST_H__


#include "QImage.h"
#include "QIHeapBuffer.h"
#include "QIThread.h"
#include "QIBuffer.h"
#include "QJPEGDMAEngine.h"
#include <stdio.h>
#include <stdlib.h>

/*===========================================================================
 * Class: QJpegDmaTest
 *
 * Description: This class represents the test application for encoder
 *
 * Notes: none
 *==========================================================================*/
class QJpegDmaTest {

public:

  /** QJpegDmaTest
   *
   *  constructor
   **/
  QJpegDmaTest();

  /** ~QJpegDmaTest
   *
   *  destructor
   **/
  ~QJpegDmaTest();

  /** Init
   *
   *  initializes the test app
   **/
  int Init();

  /** Read
   *
   *  read data from the file and fill the buffers
   **/
  int Read();

  /** Write
   *
   *  write data to the file
   **/
  int Write();

  /** Start
   *
   *  start transfer
   **/
  int Start();

  /** mpDma
   *
   *  dma object
   **/
  QJPEGDMAEngine *mpDma;

  /** mOutputFilename
   *
   *  output filename
   **/
  char *mOutputFilename;

  /** mInputFilename
   *
   *  input filename
   **/
  char *mInputFilename;

  /** mInput
   *
   *  input image object
   **/
  QImage *mInput;

  /** mOutput
   *
   *  output image object
   **/
  QImage *mOutput;

  /** mInputdata
   *
   *  input buffer
   **/
  QIBuffer *mInputdata;

  /** mOutputdata
   *
   *  output buffer
   **/
  QIBuffer *mOutputdata;

  /** mSize
   *
   *  image size
   **/
  QISize mSize;

  /** mOutSize
   *
   *  image size
   **/
  QISize mOutSize;

  /** mSS
   *
   *  image subsampling
   **/
  QISubsampling mSS;

  /** mImageSize
   *
   *  total image size
   **/
  uint32_t mImageSize;

  /** mImageOutSize
   *
   *  total image size
   **/
  uint32_t mImageOutSize;

  /** mOutputSize
   *
   *  output bitstream size
   **/
  uint32_t mOutputSize;

  /** mMutex
   *
   *  mutex variable object
   **/
  pthread_mutex_t mMutex;

  /** mCond
   *
   *  conditional variable object
   **/
  pthread_cond_t mCond;

  /** mError
   *
   *  last error value
   **/
  int mError;

  /** mFormat
   *
   *  image format
   **/
  QIFormat mFormat;

  /** mtimeout
   *
   *  waitforCompletion timeout value
   **/
  uint32_t mTimeout;

  /** mTimeoutFlag
   *
   *  flag to indicate if timeout set
   **/
  uint32_t mTimeoutFlag;

  /** mCrop
   *
   *  Crop offsets
   **/
  QICrop mCrop;

  /** mCropFlag
   *
   *  Crop enable flag
   **/
  bool mCropFlag;
};

#endif //__QJPEGDMA_TEST_H__
