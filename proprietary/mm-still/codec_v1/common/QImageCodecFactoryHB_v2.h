/*****************************************************************************
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/

#ifndef __QIMAGECODECFACTORY_HB_V2_H__
#define __QIMAGECODECFACTORY_HB_V2_H__

#include "QImageCodecFactory.h"

/*===========================================================================
 * Class: QImageCodecFactoryHB_v2
 *
 * Description: This class represents the codec factory for Honey Badger
 *             2nd level (8998 and above) family chipsets
 *
 *
 * Notes: none
 *==========================================================================*/
class QImageCodecFactoryHB_v2 : public QImageCodecFactory
{
public:

  /** QImageCodecFactoryHB_v2:
   *
   *  constructor
   **/
  QImageCodecFactoryHB_v2();

  /** ~QImageCodecFactoryHB_v2:
   *
   *  constructor
   **/
  virtual ~QImageCodecFactoryHB_v2();

  /** CreateEncoder:
   *  @aPref - codec preference
   *  @aParams - encode parameters
   *  @aCodecExclusive - exclusive flag
   *  creates the encoder and returns the encoder object to the
   *  client
   **/
  QImageEncoderInterface *CreateEncoder(QCodecPrefType aPref,
    QIEncodeParams &aParams, bool aCodecExclusive = false);

  /** CreateDecoder:
   *  @aPref - codec preference
   *  @aParams - encode parameters
   *
   *  creates the decoder and returns the decoder object to the
   *  client
   **/
  QImageDecoderInterface *CreateDecoder(QCodecPrefType aPref,
    QIDecodeParams &aParams);
};

#endif //__QIMAGECODECFACTORY_HB_V2_H__
