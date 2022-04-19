/*****************************************************************************
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/

#include "QImageCodecFactoryHB_v2.h"
#include "QImageSWEncoder.h"
#include "QImageHWDecoder10.h"
#include "QImageHWEncoder10.h"

/*===========================================================================
 * Function: QImageCodecFactoryHB_v2
 *
 * Description: QImageCodecFactoryHB_v2 constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageCodecFactoryHB_v2::QImageCodecFactoryHB_v2()
{
}

/*===========================================================================
 * Function: ~QImageCodecFactoryHB_v2
 *
 * Description: QImageCodecFactoryHB_v2 destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageCodecFactoryHB_v2::~QImageCodecFactoryHB_v2()
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
QImageEncoderInterface* QImageCodecFactoryHB_v2::CreateEncoder(
  QCodecPrefType aPref, QIEncodeParams &aParams, bool aCodecExclusive)
{
  QImageEncoderInterface *lpEncoder = NULL;
  switch (aPref) {
  case SW_CODEC_PREF:
    lpEncoder = QImageSWEncoder::New(aParams);
    if (NULL == lpEncoder) {
      QIDBG_HIGH("%s:%d] Switch to HW", __func__, __LINE__);
      lpEncoder = QImageHW10Encoder::New(aParams);
    }
    break;
  case HW_CODEC_PREF:
    lpEncoder = QImageHW10Encoder::New(aParams, aCodecExclusive);
    QIDBG_MED("%s:%d] HW JPEG Encoder Used", __func__, __LINE__);
    if (NULL == lpEncoder) {
      QIDBG_HIGH("%s:%d] HW Encoder Unavilable, Use Software Encoder",
        __func__, __LINE__);
      lpEncoder = QImageSWEncoder::New(aParams);
    }
    break;
  case SW_CODEC_ONLY:
    lpEncoder = QImageSWEncoder::New(aParams);
    QIDBG_LOW("%s:%d] SW JPEG Encoder Used", __func__, __LINE__);
    break;
  case HW_CODEC_ONLY:
    QIDBG_MED("%s:%d] HW JPEG Encoder Used", __func__, __LINE__);
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
QImageDecoderInterface* QImageCodecFactoryHB_v2::CreateDecoder(
  QCodecPrefType aPref __unused,
  QIDecodeParams &aParams __unused)
{
  return NULL;
}

