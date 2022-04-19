/*****************************************************************************
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/

#ifndef __QIMAGECODECFACTORY_BR_H__
#define __QIMAGECODECFACTORY_BR_H__

#include "QImageCodecFactory.h"

/*===========================================================================
 * Class: QImageCodecFactoryBR
 *
 * Description: This class represents the codec factory for Bear family chipsets
 *
 *
 * Notes: none
 *==========================================================================*/
class QImageCodecFactoryBR : public QImageCodecFactory
{
public:

  /** QImageCodecFactoryBR:
   *
   *  constructor
   **/
  QImageCodecFactoryBR();

  /** ~QImageCodecFactoryBR:
   *
   *  destructor
   **/
  virtual ~QImageCodecFactoryBR();

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

#endif //__QIMAGECODECFACTORY_BR_H__
