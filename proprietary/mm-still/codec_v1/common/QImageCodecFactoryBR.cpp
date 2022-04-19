/*****************************************************************************
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/

#include "QImageCodecFactoryBR.h"
#include "QImageSWEncoder.h"
#include "QImageHWDecoder10.h"
#include "QImageHWEncoder10.h"
#include "QImageQ6Encoder.h"
#include "QImageHybridEncoder.h"

/*===========================================================================
 * Function: QImageCodecFactoryBR
 *
 * Description: QImageCodecFactoryBR constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageCodecFactoryBR::QImageCodecFactoryBR()
{
}

/*===========================================================================
 * Function: ~QImageCodecFactoryBR
 *
 * Description: QImageCodecFactoryBR destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageCodecFactoryBR::~QImageCodecFactoryBR()
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
QImageEncoderInterface* QImageCodecFactoryBR::CreateEncoder(
  QCodecPrefType aPref, QIEncodeParams &aParams, bool aCodecExclusive)
{
  QImageEncoderInterface *lpEncoder = NULL;
  switch (aPref) {
  case SW_CODEC_PREF:
    lpEncoder = QImageHybridEncoder::New(aParams);
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
      QIDBG_HIGH("%s:%d] Use Hybrid JPEG encoder", __func__, __LINE__);
      //lpEncoder = QImageSWEncoder::New(aParams);
      lpEncoder = QImageHybridEncoder::New(aParams);
    }
    break;
  case SW_CODEC_ONLY:
    //lpEncoder = QImageSWEncoder::New(aParams);
    lpEncoder = QImageHybridEncoder::New(aParams);
    QIDBG_LOW("%s:%d] Hybrid JPEG Encoder Used",
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
QImageDecoderInterface* QImageCodecFactoryBR::CreateDecoder(
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

