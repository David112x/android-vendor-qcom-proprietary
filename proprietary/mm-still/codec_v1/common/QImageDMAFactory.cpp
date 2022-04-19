/*****************************************************************************
 * Copyright (c) 2016 Qualcomm Technologies, Inc.                            *
 * All Rights Reserved.                                                      *
 * Confidential and Proprietary - Qualcomm Technologies, Inc.                *
*****************************************************************************/

#include "QImageDMAFactory.h"
#include "QJPEGDMAEngine.h"
#include "QJPEGDMA_V4L2_Engine.h"

/*===========================================================================
 * Function: QImageDMAFactory
 *
 * Description: QImageCodecFactory constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageDMAFactory::QImageDMAFactory()
{
}

/*===========================================================================
 * Function: ~QImageDMAFactory
 *
 * Description: QImageDMAFactory destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QImageDMAFactory::~QImageDMAFactory()
{
}

/*===========================================================================
 * Function: CreateDMA
 *
 * Description: creates an DMA object and pass it to the client
 *
 * Input parameters:
 *   aPrivData - private data
 *   aCallback - callback function
 *
 * Return values:
 *   pointer to DMA interface
 *
 * Notes: none
 *==========================================================================*/
QImageDMAInterface* QImageDMAFactory::CreateDMA(void *aPrivData,
  DmaEngineCallback aCallback)
{
  QImageDMAInterface *lpDMA = NULL;

  if (JPEG_USE_DMA_V4L2) {
    lpDMA = QJPEGDMA_V4L2_Engine::New(aPrivData, aCallback);
    QIDBG_HIGH("%s:%d] Use V4L2 dma driver", __func__, __LINE__);
  } else {
    lpDMA = QJPEGDMAEngine::New(aPrivData, aCallback);
    QIDBG_HIGH("%s:%d] Use legacy dma driver", __func__, __LINE__);
  }

  if (NULL == lpDMA) {
    QIDBG_HIGH("%s:%d] Failed to get DMA object", __func__, __LINE__);
  }

  return lpDMA;
}


