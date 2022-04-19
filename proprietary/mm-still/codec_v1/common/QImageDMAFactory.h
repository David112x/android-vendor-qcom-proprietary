/*****************************************************************************
* Copyright (c) 2016 Qualcomm Technologies, Inc.                             *
* All Rights Reserved.                                                       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIMAGEDMAFACTORY_H__
#define __QIMAGEDMAFACTORY_H__

#include "QImageDMAInterface.h"

/*===========================================================================
 * Class: QImageDMAFactory
 *
 * Description: This class represents the base class for the DMA factory
 *
 *
 * Notes: none
 *==========================================================================*/
class QImageDMAFactory {

public:

  /** QImageDMAFactory:
   *
   *  virtual constructor for the image DMA factory base class
   *
   **/
  QImageDMAFactory();

  /** ~QImageDMAFactory:
   *
   *  virtual destructor for the image DMA factory base class
   *
   **/
  virtual ~QImageDMAFactory();

  /** CreateDMA:
   *  @aPrivData - user private data
   *  @aCallback - DMA engine callback
   *
   *  creates the DMA and returns the DMA object to the
   *  client
   **/
  virtual QImageDMAInterface *CreateDMA(void *aPrivData,
    DmaEngineCallback aCallback);
};

#endif //__QIMAGEDMAFACTORY_H__
