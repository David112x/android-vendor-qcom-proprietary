/*****************************************************************************
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/

#include "QImageCodecFactoryB.h"
#include "QImageSWEncoder.h"
#include "QImageHWDecoder10.h"
#include "QImageHWEncoder10.h"
#include "QImageQ6Encoder.h"
#include "QImageHybridEncoder.h"

/*===========================================================================
 * Function: QImageCodecFactoryB
 *
 * Description: QImageCodecFactoryB constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageCodecFactoryB::QImageCodecFactoryB()
{
}

/*===========================================================================
 * Function: ~QImageCodecFactoryB
 *
 * Description: QImageCodecFactoryB destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageCodecFactoryB::~QImageCodecFactoryB()
{
}

/*===========================================================================
 * Function: CreateEncoder
 *
 * Description: creates an encoder object and pass it to the client
 *
 * Input parameters:
 *   aPref - preference type
 *   aParams - encode parameters
 *   aCodecExclusive - exclusive flag
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageEncoderInterface* QImageCodecFactoryB::CreateEncoder(
  QCodecPrefType aPref, QIEncodeParams &aParams, bool aCodecExclusive)
{
  QImageEncoderInterface *lpEncoder = NULL;
  switch (aPref) {
  case SW_CODEC_PREF:
    lpEncoder = QImageSWEncoder::New(aParams);
    if (NULL == lpEncoder) {
      QIDBG_HIGH("%s:%d] Use hardware encoder", __func__, __LINE__);
      lpEncoder = QImageHW10Encoder::New(aParams);
    }
    break;
  case HW_CODEC_PREF:
#ifdef JPEG_USE_QDSP6_ENCODER
    lpEncoder = QImageHybridEncoder::New(aParams);
#else
    lpEncoder = QImageHW10Encoder::New(aParams, aCodecExclusive);
    QIDBG_LOW("%s:%d] HW JPEG Encoder Used", __func__, __LINE__);
#endif
    if (NULL == lpEncoder) {
      QIDBG_HIGH("%s:%d] HW Encoder Unavilable, Use Software Encoder",
        __func__, __LINE__);
      lpEncoder = QImageSWEncoder::New(aParams);
    }
    break;
  case SW_CODEC_ONLY:
    lpEncoder = QImageSWEncoder::New(aParams);
    QIDBG_LOW("%s:%d] SW JPEG Encoder Used",
      __func__, __LINE__);
    break;
  case HW_CODEC_ONLY:
    lpEncoder = QImageHW10Encoder::New(aParams, aCodecExclusive);
    break;
  default:;
  }
  return lpEncoder;
}

/*===========================================================================
 * Function: CreateDecoder
 *
 * Description: creates an decoder object and pass it to the client
 *
 * Input parameters:
 *   aPref - preference type
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageDecoderInterface* QImageCodecFactoryB::CreateDecoder(
  QCodecPrefType aPref, QIDecodeParams &aParams)
{
  QImageDecoderInterface *lpDecoder = NULL;
  switch (aPref) {
  case SW_CODEC_PREF:
    break;
  case HW_CODEC_PREF:
    break;
  case SW_CODEC_ONLY:
    break;
  case HW_CODEC_ONLY:
    lpDecoder = QImageHW10Decoder::New(aParams);
    break;
  default:;
  }
  return lpDecoder;
}

