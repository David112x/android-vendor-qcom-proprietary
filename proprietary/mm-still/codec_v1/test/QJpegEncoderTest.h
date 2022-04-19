/*****************************************************************************
 * Copyright (c) 2012-2013,2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/

#ifndef __QJPEGENCODER_TEST_H__
#define __QJPEGENCODER_TEST_H__

#include "QImageCodecFactoryBR.h"
#include "QImageCodecFactoryA.h"
#include "QImageCodecFactoryB.h"
#include "QEncodeParams.h"
#include "QImage.h"
#include "QImageEncoderObserver.h"
#include "QIHeapBuffer.h"
#include "QIThread.h"
#include "QIBuffer.h"
#include "QExifComposerParams.h"
#include "QExifComposer.h"
#include "QIONBuffer.h"
#include <stdio.h>
#include <stdlib.h>

/*===========================================================================
 * Class: QJpegEncoderTest
 *
 * Description: This class represents the test application for encoder
 *
 * Notes: none
 *==========================================================================*/
class QJpegEncoderTest : public QImageEncoderObserver, QImageWriterObserver {

public:

  /** QJpegEncoderTest
   *
   *  constructor
   **/
  QJpegEncoderTest();

  /** OutputFragment
   *  @aBuffer: output buffer
   *
   *  callback when the encoding is completed for one buffer. This
   *  function is not used
   **/
  int OutputFragment(QIBuffer &aBuffer);

  /** EncodeError
   *  @aErrorType: error type
   *
   *  callback when the encoding error is incurred
   **/
  int EncodeError(EncodeErrorType aErrorType);

  /** EncodeComplete
   *  @aOutputImage: image object
   *
   *  callback when the encoding is completed
   **/
  int EncodeComplete(QImage *aOutputImage);

  /** WriteComplete
   *  @aBuffer: buffer which is filled by the composer
   *
   *  Callback issued when the image header is written
   *  successfully into the buffer
   **/
  void WriteComplete(QIBuffer &aBuffer);

  /** WriteFragmentDone
   *  @aBuffer: buffer which is filled by the composer
   *
   *  Callback issued when the buffer passed to the composer is
   *  written and is insufficient.
   *  Note that if this callback is issued, the client should call
   *  addBuffer to send more buffers
   **/
  void WriteFragmentDone(QIBuffer &aBuffer);

  /** WriteError
   *  @aErrorType: Error type
   *
   *  Callback issued when error is occured during composition
   **/
  void WriteError(ErrorType aErrorType);

  /** ~QJpegEncoderTest
   *
   *  destructor
   **/
  ~QJpegEncoderTest();

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
   *  start encoder
   **/
  int Start();

  /** StartExifComposer
   *
   *  start exif composer
   **/
  int StartExifComposer();

  /** mFactory
   *
   *  codec factory
   **/
#ifdef CODEC_B
#ifdef CODEC_BR
  QImageCodecFactoryBR mFactory;
#elif defined CODEC_HBV2
  QImageCodecFactoryHB_v2 mFactory;
#else
  QImageCodecFactoryB mFactory;
#endif
#else
  QImageCodecFactoryA mFactory;
#endif

  /** mpEncoder
   *
   *  encoder object
   **/
  QImageEncoderInterface *mpEncoder;

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

  /** mEncodeParams
   *
   *  encode paramters
   **/
  QIEncodeParams mEncodeParams;

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

  /** mSS
   *
   *  image subsampling
   **/
  QISubsampling mSS;

  /** m_numOfPlanes
   *
   *  number of planes for input buffer
   **/
  uint32_t m_numOfPlanes;

  /** m_numOfComponents
   *
   *  number of color components for input buffer
   **/
  uint32_t m_numOfComponents;

  /** mImageSize
   *
   *  total image size
   **/
  uint32_t mImageSize;

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

  /** mQuality
   *
   *  image quality
   **/
  uint32_t mQuality;

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

  /** mJpegeName
   *
   *  choose encoder
   **/
  uint32_t mJpegeName;

  /** mComposer
   *
   *  exif composer object
   **/
  QExifComposer *mComposer;

  /** mExifParams
   *
   *  exif composer parameters
   **/
  QExifComposerParams mExifParams;

  /** m_pEncoderThread
   *
   *  Jpeg encoder QIThread to be passed to QImageEncoder
   **/
  QIThread *m_pEncoderThread;
};

#endif //__QJPEGENCODER_TEST_H__
