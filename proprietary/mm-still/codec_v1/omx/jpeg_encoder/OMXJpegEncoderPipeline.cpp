/*******************************************************************************
* Copyright (c) 2012 - 2017, 2019-2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*******************************************************************************/

#include "OMXJpegEncoderPipeline.h"
#include <cutils/properties.h>
#include <linux/msm_ion.h>

#define TMB_OUT_MAX_SZ ((640 * 480 * 3) / 2)
#define DMA_MEMCPY_WIDTH_ALIGN (256)
#define DMA_MEMCPY_HEIGHT_ALIGN (8)

/*==============================================================================
* Function : OMXJpegEncoderPipeline
* Parameters: None
* Return Value : None
* Description: Constructor
==============================================================================*/
OMXJpegEncoderPipeline::OMXJpegEncoderPipeline()
{
  m_jpegDma = NULL;
  m_jpegDmaCpy = NULL;
  m_jpegDmaOutBuf = NULL;
  m_jpegOutMem = NULL;
  m_InDmaImage = NULL;
  m_OutDmaImage = NULL;
  m_DmaMemCopyIn = NULL;
  m_DmaMemCopyOut = NULL;
  m_UseDmaScale = false;
  m_UseDmaCrop = false;
  m_SerializeEncoders = false;
  m_UseSWEncoder = false;
  m_DmaState = JPEG_DMA_IDLE;
  m_pEncoderThread = new QIThread();
  m_ForceHWThumbnail = false;
  mSeqNo = 0;
  m_MultiJpegSequence.image_type = QOMX_JPEG_IMAGE_TYPE_JPEG;
  m_DmaMaxDownScale = 0.0;
}

/*==============================================================================
* Function : ~OMXJpegEncoderPipeline
* Parameters: None
* Return Value : None
* Description: Destructor
==============================================================================*/
OMXJpegEncoderPipeline::~OMXJpegEncoderPipeline()
{
  if (m_DmaMemCopyOut) {
    delete(m_DmaMemCopyOut);
    m_DmaMemCopyOut = NULL;
  }
  if (m_DmaMemCopyIn) {
    delete(m_DmaMemCopyIn);
    m_DmaMemCopyIn = NULL;
  }
  if (m_OutDmaImage) {
    delete(m_OutDmaImage);
    m_OutDmaImage = NULL;
  }
  if (m_InDmaImage) {
    delete m_InDmaImage;
    m_InDmaImage = NULL;
  }
  if (m_jpegOutMem) {
    delete m_jpegOutMem;
    m_jpegOutMem = NULL;
  }
  if (m_jpegDmaOutBuf) {
    delete m_jpegDmaOutBuf;
    m_jpegDmaOutBuf = NULL;
  }

  if (JPEG_USE_DMA_V4L2) {
    if (m_jpegDma) {
      delete m_jpegDma;
      m_jpegDma = NULL;
    }
    if (m_jpegDmaCpy) {
      delete m_jpegDmaCpy;
      m_jpegDmaCpy = NULL;
    }
  } else {
    if (m_jpegDma) {
      delete m_jpegDma;
      m_jpegDma = NULL;
      m_jpegDmaCpy = NULL;
    }
  }

  if (m_pEncoderThread) {
    delete m_pEncoderThread;
    m_pEncoderThread = NULL;
  }
  if (mThumbBuffer) {
    delete mThumbBuffer;
    mThumbBuffer = NULL;
  }
  if (m_outThumbImage) {
    delete(m_outThumbImage);
    m_outThumbImage = NULL;
  }
}

/*==============================================================================
* Function : getInstance
* Parameters: None
* Return Value : void * - pointer to the class object
* Description: C function that returns an instance of the class.
==============================================================================*/
extern "C" void *getInstance()
{
  void *lobj = new OMXJpegEncoderPipeline();
  QIDBG_MED("%s:%d] Component ptr is %p", __func__, __LINE__, lobj);
  return lobj;
}

/*==============================================================================
* Function : configureEncodedata
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Configure the encode parmeters
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::configureEncodedata()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  uint32_t l_imageSize;
  QICrop l_crop;
  uint32_t i = 0;
  bool l_isTableSet = true;

  //Start configuration only if the abort flag is not set
  m_inputSize.setHeight((int)m_inPort->format.image.nFrameHeight);
  m_inputSize.setWidth((int)m_inPort->format.image.nFrameWidth);
  m_inputPadSize.setHeight((int)m_inPort->format.image.nSliceHeight);
  m_inputPadSize.setWidth((int)m_inPort->format.image.nStride);

  //Translate from OMX format to QIformat
  lret = translateFormat(m_inPort->format.image.eColorFormat, &m_format,
    &m_subsampling);
  if (lret != OMX_ErrorNone) {
    return lret;
  }

  //Set num of planes
  m_mainEncodeParams.setNumOfComponents(m_numOfComponents);

  l_imageSize = QImage::getImageSize(m_inputSize, m_subsampling, m_format);
  QIDBG_LOW("%s:%d] ImageSize %d %d", __func__, __LINE__, l_imageSize,
    m_inputSize.Length());

  //Create the exif composer
  if (NULL == m_composer) {
    m_composer = QExifComposer::New(*this);
    if (m_composer == NULL) {
      QIDBG_ERROR("%s:%d] failed to create exif composer", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }
  }

  //Set Crop Info
  if ((m_inputScaleInfo.nHeight) != 0 && (m_inputScaleInfo.nWidth != 0) ) {
    l_crop.setCrop((int)m_inputScaleInfo.nLeft, (int)m_inputScaleInfo.nTop,
      (int)(m_inputScaleInfo.nLeft + m_inputScaleInfo.nWidth),
      (int)(m_inputScaleInfo.nTop + m_inputScaleInfo.nHeight));
    m_mainEncodeParams.setCrop(l_crop);
  }

  /*Set output size _ if scale is enabled set the output
  size to be size after scaling*/
  if ((m_outputScaleInfo.nHeight != 0) && (m_outputScaleInfo.nWidth != 0)) {
    m_outputSize[0].setWidth((int)m_outputScaleInfo.nWidth);
    m_outputSize[0].setHeight((int)m_outputScaleInfo.nHeight);
    QIDBG_LOW("%s:%d] Scaling enabled o/p width: %d o/p height:%d",
      __func__, __LINE__, m_outputSize[0].Width(), m_outputSize[0].Height());
  } else {
    m_outputSize[0] = m_inputSize;
    QIDBG_LOW("%s:%d] Scaling not enabled width: %d height:%d",
      __func__, __LINE__, m_outputSize[0].Width(), m_outputSize[0].Height());
  }

  QIDBG_HIGH("%s:%d] size %dx%d pad %dx%d out %ux%u subsampling %d format %d",
    __func__, __LINE__,
    m_inputSize.Width(), m_inputSize.Height(),
    m_inputPadSize.Width(), m_inputPadSize.Height(),
    m_outputSize[0].Width(), m_outputSize[0].Height(),
    m_subsampling, m_format);

  m_mainEncodeParams.setOutputSize(m_outputSize[0]);
  m_mainEncodeParams.setInputSize(m_inputSize);
  m_mainEncodeParams.setRotation((uint32_t)m_rotation.nRotation);
  m_mainEncodeParams.setQuality((uint32_t)m_qualityfactor.nQFactor);
  m_mainEncodeParams.setRestartInterval(getRestartInterval(m_mainEncodeParams));

  while (i < QIQuantTable::QTABLE_MAX) {
    if (m_quantTable[i].eQuantizationTable == OMX_IMAGE_QuantizationTableMax) {
      l_isTableSet = false;
      QIDBG_MED("%s:%d] Qtable %d", __func__, __LINE__, i);
      break;
    }
    i++;
  }
  if (l_isTableSet) {
    QIDBG_MED("%s:%d] Need to set Qtable", __func__, __LINE__);

    QIQuantTable qtable[QIQuantTable::QTABLE_MAX];

    for (i = 0; i < QIQuantTable::QTABLE_MAX; i++) {
      if (m_quantTable[i].eQuantizationTable ==
        OMX_IMAGE_QuantizationTableLuma) {
        qtable[i].setType(QIQuantTable::QTABLE_LUMA);
      } else if (m_quantTable[i].eQuantizationTable ==
        OMX_IMAGE_QuantizationTableChroma) {
        qtable[i].setType(QIQuantTable::QTABLE_CHROMA);
      } else {
        return OMX_ErrorBadParameter;
      }
      uint16_t table_val[QUANT_SIZE];
      for (int j = 0; j < QUANT_SIZE; j++) {
        table_val[j] = m_quantTable[i].nQuantizationMatrix[j];
      }
      qtable[i].setTableValue(table_val);
    }
    if (QI_SUCCESS != m_mainEncodeParams.setQuantTables(qtable)) {
      QIDBG_ERROR("%s:%d] set qtable failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }
    qtable[QIQuantTable::QTABLE_LUMA].dumpQTable();
    qtable[QIQuantTable::QTABLE_CHROMA].dumpQTable();
    if (QI_SUCCESS != m_mainEncodeParams.setDefaultHuffTables()) {
      QIDBG_ERROR("%s:%d] set hufftable failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }
  } else {
    QIDBG_MED("%s:%d] Use default tables", __func__, __LINE__);
    if (QI_SUCCESS !=
      m_mainEncodeParams.setDefaultTables(m_mainEncodeParams.Quality())) {
      QIDBG_ERROR("%s:%d] set qtable failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }
  }
  if (m_huffmanTable.eHuffmanTable) {
    //ToDo: Uncomment and assign appropriately
    //m_encodeParams.setHuffTables(m_huffmanTable);
  }
  m_mainEncodeParams.setHiSpeed(m_JpegSpeedMode.speedMode ==
    QOMX_JPEG_SPEED_MODE_HIGH);

  QIDBG_MED("%s:%d] ", __func__, __LINE__);

  return lret;
}

/*==============================================================================
* Function : getRestartInterval
* Parameters: m_params
* Return Value : uint32_t
* Description: gets the restart Interval value. Hybrid Q6/SW based encoder
*              requires restart marker to be inserted at every MCUrow.
*              On the contrary, HW encoder does not use restart interval.
*              JPEG_USE_QDSP6_ENCODER is used to differentiate.
==============================================================================*/
uint32_t OMXJpegEncoderPipeline::getRestartInterval (
  QIEncodeParams& m_params __unused)
{
#ifdef JPEG_USE_QDSP6_ENCODER
  uint32_t lmcuwidth;
  switch(m_subsampling){
    case QI_H1V1:
    case QI_H1V2:
      lmcuwidth = 8;
      break;
    case QI_H2V1:
    case QI_H2V2:
    default:
      lmcuwidth = 16;
      break;
  }
  if((90 == m_params.Rotation()) || (270 == m_params.Rotation())) {
    return ((m_params.InputSize().Height() + lmcuwidth - 1)/lmcuwidth);
  } else {
    return ((m_params.InputSize().Width() + lmcuwidth - 1)/lmcuwidth);
  }
#else
  QI_UNUSED(m_params);
  return 0;
#endif
}

/*==============================================================================
* Function : configureEncodedata
* Parameters: a_inBuffer, a_outBuffer
* Return Value : OMX_ERRORTYPE
* Description: Configure the input and output buffers
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::configureBuffers(
  OMX_BUFFERHEADERTYPE *a_inBuffer,
  OMX_BUFFERHEADERTYPE *a_outBuffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QOMX_BUFFER_INFO *lInBufferInfo = NULL;
  QOMX_BUFFER_INFO *lOutBufferInfo = NULL;

  lInBufferInfo = reinterpret_cast<QOMX_BUFFER_INFO *>
    (a_inBuffer->pOutputPortPrivate);
  m_inputQIBuffer = new QIBuffer((uint8_t*)a_inBuffer->pBuffer,
    a_inBuffer->nAllocLen);
  if (!m_inputQIBuffer) {
    QIDBG_ERROR("%s:%d] Error allocating i/p QIBuffer", __func__, __LINE__);
  return OMX_ErrorInsufficientResources;
  }
  m_inputQIBuffer->SetFd((int)lInBufferInfo->fd);
  m_inputQIBuffer->SetOffset(lInBufferInfo->offset);
  m_inputQIBuffer->SetFilledLen(a_inBuffer->nFilledLen);

  lOutBufferInfo = reinterpret_cast<QOMX_BUFFER_INFO *>
    (a_outBuffer->pOutputPortPrivate);
  QIDBG_MED("%s: %d: O/p buffer address is %p, size: %d", __func__, __LINE__,
    a_outBuffer->pBuffer, (int)a_outBuffer->nAllocLen);
  m_outputQIBuffer = new QIBuffer((uint8_t*)a_outBuffer->pBuffer,
    a_outBuffer->nAllocLen);
  if (!m_outputQIBuffer) {
    QIDBG_ERROR("%s:%d] Error allocating o/p QIBuffer", __func__, __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  m_outputQIBuffer->SetFd(lOutBufferInfo->fd);
  m_outputQIBuffer->SetOffset(lOutBufferInfo->offset);
  m_outputQIBuffer->SetFilledLen(0);

  QIDBG_MED("%s:%d] MAIN buf=%p, fd=%u, offset=%u filled=%u", __func__, __LINE__,
    a_inBuffer->pBuffer,
    lInBufferInfo->fd,
    lInBufferInfo->offset,
    a_inBuffer->nFilledLen);

  QIDBG_MED("%s:%d] Output buf=%p, fd=%u, offset=%u filled=%u", __func__,
    __LINE__, a_outBuffer->pBuffer,
    lOutBufferInfo->fd,
    lOutBufferInfo->offset,
    0);

  return lret;

}

/*==============================================================================
* Function : configureTmbBuffer
* Parameters: a_inBuffer, a_outBuffer
* Return Value : OMX_ERRORTYPE
* Description: Configure input thumbnail buffer
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::configureTmbBuffer(
  OMX_BUFFERHEADERTYPE *a_inTmbBuffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QOMX_BUFFER_INFO *lInBufferInfo = NULL;
  QOMX_BUFFER_INFO *lOutBufferInfo = NULL;

  lInBufferInfo = reinterpret_cast<QOMX_BUFFER_INFO *>
    (a_inTmbBuffer->pOutputPortPrivate);
  m_inputQTmbBuffer = new QIBuffer((uint8_t*)a_inTmbBuffer->pBuffer,
      a_inTmbBuffer->nAllocLen);
  if (!m_inputQTmbBuffer) {
    QIDBG_ERROR("%s:%d] Error allocating i/p QIBuffer", __func__, __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  m_inputQTmbBuffer->SetFd((int)lInBufferInfo->fd);
  m_inputQTmbBuffer->SetOffset(lInBufferInfo->offset);
  m_inputQTmbBuffer->SetFilledLen(a_inTmbBuffer->nFilledLen);

  QIDBG_MED("%s:%d] TMB buf=%p, fd=%u, offset=%u, filled=%u",
    __func__, __LINE__,
    a_inTmbBuffer->pBuffer,
    lInBufferInfo->fd,
    lInBufferInfo->offset,
    a_inTmbBuffer->nFilledLen);

  return lret;
}

/*==============================================================================
* Function : writeExifData
* Parameters: QImage *aThumbnail - The thumbnail buffer after encoding. Null
* when thumbnail is not encoded.
* Return Value : OMX_ERRORTYPE
* Description: Add the exif data and encoded thumbnail to the o/p buffer
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::writeExifData(QImage *aThumbnail,
  QIBuffer *aOutputBuffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lrc = 0;
  int mobicat_mask = 0;
  int app2_header_len = 0, mpo_header_len = 0;
  QExifComposerParams::MutiImageInfo mp_info;
  char *mi_payload = NULL;

#ifdef PROCESS_METADATA
  lret = processMetadata();

  if (OMX_ErrorNone != lret) {
    QIDBG_ERROR("%s:%d] processMetadata failed", __func__, __LINE__);
    return lret;
  }
#endif

  if (m_Metadata.metadata != NULL) {
    mobicat_mask = m_Metadata.mobicat_mask;
  } else {
    QIDBG_HIGH("%s:%d] Metadata pointer is NULL, disable mobicat composing",
      __func__, __LINE__);
  }

  //Set the 3A sw version info
  m_exifParams.Set3ASWVer(m_sw3aVersion);

  //enable mobicat
  QIDBG_HIGH("%s:%d] Image type %d mobicat_mask %x", __func__,
    __LINE__, m_MultiJpegSequence.image_type, mobicat_mask);
  if ((mobicat_mask > 0) ||
    (m_MultiJpegSequence.image_type == QOMX_JPEG_IMAGE_TYPE_MPO) ||
    m_MultiJpegSequence.enable_metadata) {
    m_mobicatComposer = new QMobicatComposer();
    if (m_mobicatComposer == NULL) {
      QIDBG_ERROR("%s:%d] failed to create mobicat composer", __func__,
        __LINE__);
      return OMX_ErrorInsufficientResources;
    }

    if (mobicat_mask & MOBICAT_MASK) {
      char* mobicatStr = m_mobicatComposer->
        ParseMobicatData(m_Metadata.metadata);
      if (mobicatStr == 0) {
        QIDBG_ERROR("%s:%d] Error in composing mobicat string",
          __func__, __LINE__);
        return OMX_ErrorUndefined;
      }
      QIDBG_HIGH("%s:%d] m_mobicat.size %d", __func__, __LINE__,
        (int) strlen(mobicatStr));

      m_exifParams.SetMobicatFlag(true);
      m_exifParams.SetMobicat(mobicatStr);
    }
    if (mobicat_mask & STATS_DEBUG_MASK) {
      char* stats_payload = m_mobicatComposer->
        Compose3AStatsPayload(m_Metadata.metadata);
      uint32_t stats_payload_size =
        m_mobicatComposer->Get3AStatsSize();

      if (stats_payload_size == 0) {
        QIDBG_ERROR("%s:%d] [MOBICAT_DBG] Stats debug payload size is 0",
          __func__, __LINE__);
      } else {
         m_exifParams.Set3AFlag(true);
         m_exifParams.Set3A(stats_payload, stats_payload_size);
      }
    }
    if ((m_MultiJpegSequence.image_type == QOMX_JPEG_IMAGE_TYPE_MPO) ||
      m_MultiJpegSequence.enable_metadata) {
      if (!m_MultiJpegSequence.is_primary_image) {
        mi_payload = m_mobicatComposer->ComposeMultiImageData(
          m_Metadata.metadata, m_Metadata.static_metadata);
      } else {
        mi_payload = m_mobicatComposer->ComposeMultiImageData(
          m_Metadata.metadata, NULL);
      }
      uint32_t mi_payload_size =  m_mobicatComposer->GetMIPayloadSize();
      if (mi_payload == 0) {
        QIDBG_ERROR("%s:%d] [MOBICAT_DBG] MPO debug payload size is 0",
          __func__, __LINE__);
      } else {
         m_exifParams.SetMIMetaFlag(true);
         m_exifParams.SetMIMetadata(mi_payload, mi_payload_size);
      }
    }
  }

  lrc = m_composer->addBuffer(aOutputBuffer);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  /*Set app2 header length if it is a MP sequence*/
  if (m_MultiJpegSequence.image_type == QOMX_JPEG_IMAGE_TYPE_MPO) {
    app2_header_len = MP_APP2_FIELD_LENGTH_BYTES;
    mpo_header_len = MP_APP2_FIELD_LENGTH_BYTES + MP_FORMAT_IDENTIFIER_BYTES +
      MP_ENDIAN_BYTES + MP_HEADER_OFFSET_TO_FIRST_IFD_BYTES;
    mp_info.image_type = QI_MPO;
    mp_info.num_of_images = m_MultiJpegSequence.num_of_images;

    QIDBG_MED("%s:%d] MPO Sequence app2_header_len %d, mpo_hdr_len %d, num_of_images %d",
      __func__, __LINE__, app2_header_len, mpo_header_len, mp_info.num_of_images);

    if (m_MultiJpegSequence.is_primary_image) {
      mpo_header_len += MP_INDEX_COUNT_BYTES + MP_INDEX_VERSION_BYTES +
      MP_INDEX_OFFSET_OF_NEXT_IFD_BYTES;
      mp_info.is_primary = TRUE;
    } else {
      mp_info.is_primary = FALSE;
    }
  } else {
    mp_info.image_type = QI_JPEG;
    mp_info.num_of_images = 1;
  }
  mp_info.large_thumbnail_present = FALSE;
  app2_header_len += mpo_header_len;
  m_exifParams.SetAppHeaderLen(app2_header_len);
  m_exifParams.SetMultiImageInfo(mp_info);

  if (aThumbnail) {
    m_exifParams.SetEncodeParams(m_thumbEncodeParams, true);
    m_exifParams.SetSubSampling(m_thumbSubsampling, true);
  }
  m_exifParams.SetEncodeParams(m_mainEncodeParams);
  m_exifParams.SetExif(&m_exifInfoObj);
  m_exifParams.SetSubSampling(m_subsampling);

  lrc = m_composer->SetParams(m_exifParams);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  lrc = m_composer->Start(aThumbnail);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }
  return lret;
}

/*==============================================================================
* Function : startEncode
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Get the encoder from the factory and start encoding
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::startEncode()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QImageCodecFactory::QCodecPrefType lCodecPref =
  QImageCodecFactory::HW_CODEC_PREF;
  int lrc = 0;
  QIBuffer *lOutBuf;

  //Set the offset for each plane
  uint32_t lOffset[OMX_MAX_NUM_PLANES] = {m_imageBufferOffset.yOffset,
    m_imageBufferOffset.cbcrOffset[0], m_imageBufferOffset.cbcrOffset[1]};

  QIDBG_MED("%s:%d] JPEG in lOffset: %d %d %d", __func__, __LINE__, lOffset[0],
    lOffset[1], lOffset[2]);

  //Set the physical offset for each plane
  uint32_t lPhyOffset[QI_MAX_PLANES] = {0,
    m_imageBufferOffset.cbcrStartOffset[0],
    m_imageBufferOffset.cbcrStartOffset[1]};

  QIDBG_MED("%s:%d] JPEG in lPhyOffset: %d %d %d", __func__, __LINE__,
    lPhyOffset[0], lPhyOffset[1], lPhyOffset[2]);

  for (int i = 0; i < 2; i++) {
    //Get the appropriate Encoder from the factory
    if (NULL == m_mainEncoder) {
      m_mainEncoder = m_factory.CreateEncoder(lCodecPref,
          m_mainEncodeParams);
      if (m_mainEncoder == NULL) {
        QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
        return OMX_ErrorInsufficientResources;
      }
    }
    m_inputMainImage = new QImage(m_inputPadSize, m_subsampling, m_format,
      m_inputSize);
    if (m_inputMainImage == NULL) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }

    lrc = m_inputMainImage->setDefaultPlanes(m_numOfPlanes,
        m_inputQIBuffer->Addr(), m_inputQIBuffer->Fd(), lOffset, lPhyOffset);
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorUndefined;
    }

    QIBuffer lIONBuffer = QIBuffer(m_IONBuffer.vaddr, m_IONBuffer.length);
    lIONBuffer.SetFilledLen(0);
    lIONBuffer.SetFd(m_IONBuffer.fd);

    lOutBuf = &lIONBuffer;

    m_outputMainImage = new QImage(lOutBuf->Addr() +
      lOutBuf->FilledLen(),
      lOutBuf->Length() - lOutBuf->FilledLen(),
      QI_BITSTREAM);

    if (m_outputMainImage == NULL) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }

    m_outputMainImage->setFd(lOutBuf->Fd());

    lrc = m_mainEncoder->SetOutputMode(QImageEncoderInterface::ENORMAL_OUTPUT);
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorUndefined;
    }

    lrc = m_mainEncoder->setEncodeParams(m_mainEncodeParams);
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorUndefined;
    }

    lrc = m_mainEncoder->addInputImage(*m_inputMainImage);
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorUndefined;
    }

    lrc = m_mainEncoder->addOutputImage(*m_outputMainImage);
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorUndefined;
    }

    lrc = m_mainEncoder->addObserver(*this);
    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorUndefined;
    }

    mSeqNo++;
    QIDBG_INFO("%s:%d] Start Encode %d", __func__, __LINE__, mSeqNo);
    ATRACE_BEGIN_SNPRINTF(30, "Camera:Jpeg:Encode%d", mSeqNo);
    ATRACE_END();
    ATRACE_INT("Camera:Jpeg:Encode", 1);
    m_mainImageEncoding = OMX_TRUE;
    lrc = m_mainEncoder->Start(m_pEncoderThread);
    if (!lrc) {
      lret = OMX_ErrorNone;
      m_mainImageEncoding = OMX_TRUE;
      break;
    } else {
      delete m_mainEncoder;
      m_mainEncoder = NULL;
      lret = OMX_ErrorUndefined;
      lCodecPref = QImageCodecFactory::SW_CODEC_ONLY;
      ATRACE_INT("Camera:Jpeg:Encode", 0);
      QIDBG_ERROR("%s:%d] Main Image encoding failed to start, "
        "switching to alternative encoder",__func__, __LINE__);

      continue;
    }
  }

  return lret;
}

/*==============================================================================
* Function : CompleteMainImage
* Parameters: QImage
* Return Value : int
* Description: Completes main image encoding.
==============================================================================*/
int OMXJpegEncoderPipeline::CompleteMainImage()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  QI_LOCK(&m_abortlock);

  QIDBG_MED("%s:%d] E get_memory %p", __func__, __LINE__, m_memOps.get_memory);
  /* If abort flag is set do nothing */
  if (!m_abort_flag && (OMX_FALSE == m_releaseFlag)) {
    //The i/p buffer has been consumed completely. Set the nFilledLen to 0x0
    m_currentInBuffHdr->nFilledLen = 0;
    if (NULL != m_memOps.get_memory) {

      bool lOutMemoryReserved;

      QIDBG_HIGH("%s:%d Allocating extra temp buffer for Exif ", __func__, __LINE__);
      uint8_t *exif_buf = (uint8_t*)calloc (getEstimatedExifSize(),sizeof(uint8_t));
      if (exif_buf == NULL) {
        QIDBG_ERROR("%s:%d exif mem alloc failed", __func__,  __LINE__);
        QI_UNLOCK(&m_abortlock);
        return QI_ERR_GENERAL;
      }
      QIBuffer lApp1Buf = QIBuffer((uint8_t*)exif_buf, getEstimatedExifSize());

      if ((m_thumbnailInfo.input_height != 0) &&
        (m_thumbnailInfo.input_width != 0)) {

        /*Invalidate the cache before memcopy of thumbnail
          buffer during Exif composition*/
        if (m_outThumbImage->Fd() >= 0) {
          QIONBuffer::DoCacheOps(m_outThumbImage->Fd(),
            m_outThumbImage->FilledLen(), QIONBuffer::CACHE_CLEAN_INVALIDATE);
        }

        lret = writeExifData(m_outThumbImage, &lApp1Buf);
      } else {
        lret = writeExifData(NULL, &lApp1Buf);
      }
      if (lret != OMX_ErrorNone) {
        QIDBG_ERROR("%s:%d ", __func__, __LINE__);
        free (exif_buf);
        goto unlock_and_exit;
      }

      omx_jpeg_ouput_buf_t *jpeg_out =
        (omx_jpeg_ouput_buf_t *)m_outputQIBuffer->Addr();

      lOutMemoryReserved = reserveJpegOutMem(lApp1Buf.FilledLen());

      if (lOutMemoryReserved == false) {
        QIDBG_ERROR("%s:%d Can not reserve output memory", __func__, __LINE__);
        free (exif_buf);
        QI_UNLOCK(&m_abortlock);
        return QI_ERR_GENERAL;
      }
      // copy the exif data to final bit stream pointer
      memcpy(jpeg_out->vaddr, exif_buf, lApp1Buf.FilledLen());
      free (exif_buf);

      QIBuffer lOutBuf = QIBuffer((uint8_t*)jpeg_out->vaddr, jpeg_out->size);
      lOutBuf.SetFd(jpeg_out->fd);

      lOutBuf.SetFilledLen(lApp1Buf.FilledLen());

      //Set the filled length of the ouput buffer
      m_currentOutBuffHdr->nFilledLen = m_outputMainImage->FilledLen() +
        lOutBuf.FilledLen();

      QIONBuffer::DoCacheOps(m_jpegOutMem, QIONBuffer::CACHE_CLEAN_INVALIDATE);
      if(JPEG_USE_NO_DMACPY) {
          // using software memcpy instead of dma memcpy.
          // As dma memcpy taking moretime than expected.
          QIDBG_MED("%s:%d] NO DMACPY and only SWMEMCPY is used", __func__, __LINE__);
          lret = startSwMemcpy(*m_outputMainImage, lOutBuf);
            if (lret != OMX_ErrorNone) {
              QIDBG_ERROR("%s:%d Start sw memcpy failed", __func__, __LINE__);
              goto unlock_and_exit;
            }
      } else {
          lret = startDmaMemcpy(*m_outputMainImage, lOutBuf);
          if (lret != OMX_ErrorNone) {
            QIDBG_ERROR("%s:%d Start dma memcpy failed switch to sw memcpy",
               __func__, __LINE__);
            lret = startSwMemcpy(*m_outputMainImage, lOutBuf);
            if (lret != OMX_ErrorNone) {
              QIDBG_ERROR("%s:%d Start sw memcpy failed", __func__, __LINE__);
              goto unlock_and_exit;
            }
          }
      }
    } else if (m_outputQIBuffer->Addr() != m_outputMainImage->BaseAddr()) {
      //Set the filled length of the ouput buffer
      m_currentOutBuffHdr->nFilledLen = m_outputMainImage->FilledLen() +
          m_outputQIBuffer->FilledLen();
      if(JPEG_USE_NO_DMACPY) {
        // using software memcpy instead of dma memcpy.
        // As dma memcpy taking moretime than expected.
        QIDBG_MED("%s:%d] NO DMACPY and only SWMEMCPY is used", __func__, __LINE__);
        lret = startSwMemcpy(*m_outputMainImage, *m_outputQIBuffer);;
        if (lret != OMX_ErrorNone) {
          QIDBG_ERROR("%s:%d Start sw memcpy failed", __func__, __LINE__);
          goto unlock_and_exit;
        }
      } else {
        //Use sw memcpy if ion fd is not available
        if (m_outputQIBuffer->Fd() < 0) {
          lret = startSwMemcpy(*m_outputMainImage, *m_outputQIBuffer);
          if (lret != OMX_ErrorNone) {
            QIDBG_ERROR("%s:%d Start sw memcpy failed", __func__,  __LINE__);
            goto unlock_and_exit;
          }
        } else {
          QIONBuffer::DoCacheOps(m_outputQIBuffer->Fd(),
            m_outputQIBuffer->FilledLen(), QIONBuffer::CACHE_CLEAN_INVALIDATE);

          lret = startDmaMemcpy(*m_outputMainImage, *m_outputQIBuffer);
          if (lret != OMX_ErrorNone) {
            QIDBG_ERROR("%s:%d Start dma memcpy failed switch to sw memcpy",
              __func__,  __LINE__);
            lret = startSwMemcpy(*m_outputMainImage, *m_outputQIBuffer);
            if (lret != OMX_ErrorNone) {
              QIDBG_ERROR("%s:%d Start sw memcpy failed", __func__,  __LINE__);
              goto unlock_and_exit;
            }
          }
        }
      }
    } else {
      int lQIret = ReleaseMainImage();
      if (lQIret != QI_SUCCESS) {
        lret = OMX_ErrorUndefined;
        QIDBG_ERROR("%s:%d ", __func__, __LINE__);
        goto unlock_and_exit;
      }
    }
  }

unlock_and_exit:
  QI_UNLOCK(&m_abortlock);
  QIDBG_MED("%s:%d] X", __func__, __LINE__);

  return lret;
}

/*==============================================================================
* Function : ReleaseMainImage
* Parameters: QImage
* Return Value : int
* Description: Send main image back to omx.
==============================================================================*/
int OMXJpegEncoderPipeline::ReleaseMainImage()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  /* Post ETB Done and FTB Done to the queue since we dont want to do a
    callback with the Event thread from the codec layer. */

  //Post EBD to the message queue
  QIMessage *lebdMessage = new QIMessage();
  if (!lebdMessage) {
    QIDBG_ERROR("%s:%d] Could not alloate QIMessage", __func__,  __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  lebdMessage->m_qMessage = OMX_MESSAGE_ETB_DONE;
  lebdMessage->pData = m_currentInBuffHdr;

  //Post FBD message to the message queue
  QIMessage *lfbdMessage = new QIMessage();
  if (!lfbdMessage) {
    delete lebdMessage;
    QIDBG_ERROR("%s:%d] Could not allocate QIMessage", __func__,  __LINE__);
    return QI_ERR_NO_MEMORY;
  }

  QIDBG_HIGH("%s:%d] Encoded image length %d", __func__, __LINE__,
    (int)m_currentOutBuffHdr->nFilledLen);

  lfbdMessage->m_qMessage = OMX_MESSAGE_FTB_DONE;
  lfbdMessage->pData = m_currentOutBuffHdr;

  lret = postMessage(lebdMessage);
  if (QI_ERROR(lret)) {
    QIDBG_ERROR("%s:%d] Could not send EBD", __func__,  __LINE__);
    delete lebdMessage;
    delete lfbdMessage;
    return QI_ERR_NO_MEMORY;
  }

  lret = postMessage(lfbdMessage);
  if (QI_ERROR(lret)) {
    QIDBG_ERROR("%s:%d] Could not send FBD", __func__,  __LINE__);
    delete lfbdMessage;
    return QI_ERR_NO_MEMORY;
  }

  // Send message for a new encode process
  QIMessage *lEncodeMessage = new QIMessage();
  if (!lEncodeMessage) {
    QIDBG_ERROR("%s:%d] Could not alloate QIMessage", __func__,  __LINE__);
    return QI_ERR_NO_MEMORY;
  }
  lEncodeMessage->m_qMessage = OMX_MESSAGE_START_NEW_ENCODE;
  lret = postMessage(lEncodeMessage);
  if (QI_ERROR(lret)) {
    QIDBG_ERROR("%s:%d] Could not send Start encode", __func__,  __LINE__);
    delete lEncodeMessage;
    return QI_ERR_NO_MEMORY;
  }

  return QI_SUCCESS;
}

/*==============================================================================
* Function : thumbnailEncodePending
* Parameters: None
* Return Value : int
* Description: Check if thumbnail encode is needed after main image encode
*
==============================================================================*/
bool OMXJpegEncoderPipeline::thumbnailEncodePending()
{
  //If release flag is set not need to encode the thumbnail
  if (OMX_TRUE == m_releaseFlag) {
    return false;
  }

  //If we dont use dma thumbnail encode is always done as part of main image
  if (FALSE == m_UseDmaScale) {
    return false;
  }

  //If DMA is still downscaling dont start encode here
  if (JPEG_DMA_DOWNSCALE == m_DmaState) {
    return false;
  }

  //If thumbnail is started or finished skip encoding
  if (OMX_TRUE == m_thumbEncoding || OMX_TRUE == m_thumbEncodingComplete) {
    return false;
  }

  return true;
}

/*==============================================================================
* Function : EncodeComplete
* Parameters: None
* Return Value : int
* Description: This function is called from the JPEG component when encoding is
* complete
==============================================================================*/
int OMXJpegEncoderPipeline::EncodeComplete(QImage *aOutputImage)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QIMessage *lmessage = NULL;
  int ret = QI_ERR_GENERAL;
  QI_LOCK(&mEncodeDoneLock);
  if ((m_thumbEncoding == OMX_TRUE) && (m_outThumbImage != NULL) &&
    m_outThumbImage->BaseAddr() == aOutputImage->BaseAddr()) {
    ATRACE_INT("Camera:Jpeg:Thumb", 0);
    QIDBG_HIGH("%s:%d] Thumbnail Encoding complete. %d",
      __func__, __LINE__, mSeqNo);
    m_thumbEncoding = OMX_FALSE;
    m_thumbEncodingComplete = OMX_TRUE;

    if (NULL == m_memOps.get_memory) {
      /*Invalidate the cache before memcopy of thumbnail
          buffer during Exif composition*/
      if (aOutputImage->Fd() >= 0) {
        QIONBuffer::DoCacheOps(aOutputImage->Fd(),
          aOutputImage->FilledLen(), QIONBuffer::CACHE_CLEAN_INVALIDATE);
      }
      lret = writeExifData(aOutputImage, m_outputQIBuffer);
      QIDBG_ERROR("%s:%d] Exif length: %d", __func__,  __LINE__,
        m_outputQIBuffer->FilledLen());
      if (QI_ERROR(lret)) {
        goto error;
      }
    }

    /* send ETB for thumbnail */
    QIMessage *lEtbMessage = new QIMessage();
    if (!lEtbMessage) {
      QIDBG_ERROR("%s:%d] Could not allocate QIMessage", __func__,  __LINE__);
      goto error_nomem;
    }
    lEtbMessage->m_qMessage = OMX_MESSAGE_ETB_DONE;
    lEtbMessage->pData = m_currentInTmbBuffHdr;
    postMessage(lEtbMessage);

    if (m_encoding_mode == OMX_Serial_Encoding) {
      /* Thumbnail exif write successful, Start main image encode */
      lmessage = new QIMessage();
      if (!lmessage) {
        QIDBG_ERROR("%s:%d] Could not allocate QIMessage", __func__,  __LINE__);
        goto error_nomem;
      }
      lmessage->m_qMessage = OMX_MESSAGE_START_MAIN_ENCODE;
      postMessage(lmessage);
      lmessage = NULL;
    } else {
      /* parallel encoding */
      QIDBG_MED("%s:%d] parallel encoding m_mainEncodingComplete %d", __func__,
        __LINE__, m_mainEncodingComplete);

      if (m_outputMainImage != NULL && m_outputMainImage->FilledLen() &&
        (OMX_TRUE == m_mainEncodingComplete)) {
        /* MainImage was finished first, now write MainImage */
        ret = CompleteMainImage();
        if (ret != QI_SUCCESS) {
          goto error;
        }
      }
    }
  } else if (m_outputMainImage != NULL &&
    m_outputMainImage->BaseAddr() == aOutputImage->BaseAddr()) {
    /* main image encoding complete */
    QIDBG_HIGH("%s:%d] MainImage Encoding complete. Filled "
      "Length = %d m_thumbEncodingComplete %d mSeqNo %d",
      __func__, __LINE__, m_outputMainImage->FilledLen(),
      m_thumbEncodingComplete, mSeqNo);
    ATRACE_INT("Camera:Jpeg:Encode", 0);
    m_mainImageEncoding = OMX_FALSE;
    m_mainEncodingComplete = OMX_TRUE;

    if (m_encoding_mode == OMX_Serial_Encoding) {
      ret = CompleteMainImage();
      if (ret != QI_SUCCESS) {
        goto error;
      }
    } else {
      /* parallel encoding */

      // Hold abort lock here is protecting abort flag
      QI_LOCK(&m_abortlock);
      if (thumbnailEncodePending()) {
        if (m_UseSWEncoder) {
          lret = startThumbnailEncode(QImageCodecFactory::SW_CODEC_ONLY);
        } else {
          lret = startThumbnailEncode(QImageCodecFactory::HW_CODEC_PREF);
        }
      }
      QI_UNLOCK(&m_abortlock);
      if (QI_ERROR(lret)) {
        goto error;
      }
      /* thumbnail does not exist OR has already been encoded.
         Write MainImage to EXIF*/
      if (!m_inTmbPort->bEnabled ||
        (m_outThumbImage != NULL && m_outThumbImage->FilledLen()
        && (OMX_TRUE == m_thumbEncodingComplete))) {
       ret =  CompleteMainImage();
       if (ret != QI_SUCCESS) {
         goto error;
       }
      }
    }
  }

  QI_UNLOCK(&mEncodeDoneLock);
  return QI_SUCCESS;

error:
  QI_UNLOCK(&mEncodeDoneLock);
  /* Propagate error */
  lmessage = new QIMessage();
  if (lmessage) {
    lmessage->m_qMessage = OMX_MESSAGE_EVENT_ERROR;
    lmessage->iData = lret;
    postMessage(lmessage);
  }
  return QI_ERR_GENERAL;

error_nomem:
  /* TBD: Propagate error */
  QI_UNLOCK(&mEncodeDoneLock);
  return QI_ERR_NO_MEMORY;
}

/*==============================================================================
* Function : EncodeError
* Parameters: None
* Return Value : int
* Description: This function is called from the JPEG component when there is an
* error in encoding
==============================================================================*/
int OMXJpegEncoderPipeline::EncodeError(EncodeErrorType aErrorType)
{
  int lrc = QI_SUCCESS;
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QIDBG_MED("%s:%d] Error %d", __func__, __LINE__, aErrorType);
  pthread_mutex_lock(&m_abortlock);
  if (!m_abort_flag) {
    m_executionComplete = OMX_TRUE;
    QIMessage *lErrMessage = new QIMessage();
    lErrMessage->m_qMessage = OMX_MESSAGE_EVENT_ERROR;
    if (ERROR_OVERFLOW == aErrorType) {
      lErrMessage->iData = OMX_ErrorOverflow;
    } else {
      lErrMessage->iData = OMX_ErrorUndefined;
    }
    lret = postMessage(lErrMessage);
    if (lret != OMX_ErrorNone) {
      QIDBG_ERROR("%s %d: Error posting message", __func__, __LINE__);
      lrc = QI_ERR_GENERAL;
    }
  }
  pthread_mutex_unlock(&m_abortlock);
  return lrc;
}

/*==============================================================================
* Function : OutputFragment
* Parameters: QIBuffer &aBuffer
* Return Value : int
* Description:
==============================================================================*/
int OMXJpegEncoderPipeline::OutputFragment(QIBuffer &aBuffer __unused)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  return QI_SUCCESS;
}

/*==============================================================================
* Function : WriteError
* Parameters: ErrorType aErrorType
* Return Value : int
* Description:
==============================================================================*/
void OMXJpegEncoderPipeline::WriteError(ErrorType aErrorType __unused)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
}
/*==============================================================================
* Function : WriteComplete
* Parameters: uint8_t *aBuffer, uint32_t aSize
* Return Value : int
* Description:
==============================================================================*/
void OMXJpegEncoderPipeline::WriteComplete(QIBuffer &aBuffer __unused)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
}

/*==============================================================================
* Function : WriteFragmentDone
* Parameters: QIBuffer &aBuffer
* Return Value : void
* Description:
==============================================================================*/
void OMXJpegEncoderPipeline::WriteFragmentDone(QIBuffer &aBuffer __unused)
{
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
}

/*==============================================================================
* Function : configureThumbnailData
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Configure the encode parameters for thumbnail
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::configureThumbnailData()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QICrop l_crop;
  bool l_UseEncoderCrop;
  uint32_t l_imageSize;
  int lrc;
  float l_tmbScale_w = 1.0f;
  float l_tmbScale_h = 1.0f;
  QISize lTmbInSize;

  m_UseDmaScale = false;
  m_UseDmaCrop = false;
  m_SerializeEncoders = false;
  l_UseEncoderCrop = false;

  lTmbInSize.setWidth((int)m_inTmbPort->format.image.nFrameWidth);
  lTmbInSize.setHeight((int)m_inTmbPort->format.image.nFrameHeight);

  //Translate from OMX format to QIformat
  lret = translateFormat(m_inTmbPort->format.image.eColorFormat, &m_thumbFormat,
    &m_thumbSubsampling);
  if (lret != OMX_ErrorNone) {
    return lret;
  }

  if ((m_thumbnailInfo.crop_info.nWidth) != 0 &&
    (m_thumbnailInfo.crop_info.nHeight != 0)) {
    lTmbInSize.setWidth(m_thumbnailInfo.crop_info.nWidth);
    lTmbInSize.setHeight(m_thumbnailInfo.crop_info.nHeight);
  }

  if ((m_thumbnailInfo.output_width != 0) &&
    (m_thumbnailInfo.output_height != 0)) {
    m_outputSize[1].setWidth((int)m_thumbnailInfo.output_width);
    m_outputSize[1].setHeight((int)m_thumbnailInfo.output_height);
    QIDBG_MED("%s:%d] Thumbnail Scaling enabled o/p width: %d o/p height:%d",
      __func__, __LINE__, m_outputSize[1].Width(), m_outputSize[1].Height());

    if ((m_jpegDma != NULL) && (lTmbInSize.Width() != m_outputSize[1].Width() ||
      lTmbInSize.Height() != m_outputSize[1].Height())) {
      if (lTmbInSize.Width() >= m_outputSize[1].Width() &&
        lTmbInSize.Height() >= m_outputSize[1].Height()) {
        m_UseDmaScale = true;
      }
    }
  } else {
    m_outputSize[1].setWidth((int)m_inputTmbSize.Width());
    m_outputSize[1].setHeight((int)m_inputTmbSize.Height());
    QIDBG_MED("%s:%d] Thumbnail Scaling not enabled width: %d height:%d",
      __func__, __LINE__, m_outputSize[1].Width(), m_outputSize[1].Height());
  }

  if (m_UseDmaScale != true) {
    QOMX_YUV_FRAME_INFO *lbufferOffset = &m_thumbnailInfo.tmbOffset;
    if (!m_inTmbPort->bEnabled) {
      lbufferOffset = &m_imageBufferOffset;
      QIDBG_ERROR("%s:%d] TMB PORT IS NOT ENABLED", __func__, __LINE__);
    }

    //Set the offset for each plane
    uint32_t lOffset[OMX_MAX_NUM_PLANES] = {lbufferOffset->yOffset,
      lbufferOffset->cbcrOffset[0] , lbufferOffset->cbcrOffset[1]};

    //Set the physical offset for each plane
    uint32_t lPhyOffset[QI_MAX_PLANES] = {0,
      lbufferOffset->cbcrStartOffset[0],
      lbufferOffset->cbcrStartOffset[1]};

    //Start configuration only if the abort flag is not set
    m_inputTmbSize.setHeight((int)m_inTmbPort->format.image.nFrameHeight);
    m_inputTmbSize.setWidth((int)m_inTmbPort->format.image.nFrameWidth);
    m_inputTmbPadSize.setHeight((int)m_inTmbPort->format.image.nSliceHeight);
    m_inputTmbPadSize.setWidth((int)m_inTmbPort->format.image.nStride);

    m_inThumbImage = new QImage(m_inputTmbPadSize, m_thumbSubsampling,
      m_thumbFormat,
      m_inputTmbSize);
    if (m_inThumbImage == NULL) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }

    lrc = m_inThumbImage->setDefaultPlanes(m_numOfPlanes,
      m_inputQTmbBuffer->Addr(),
      m_inputQTmbBuffer->Fd(), lOffset, lPhyOffset);

    if (lrc) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorUndefined;
    }
  } else {
    m_inputTmbSize.setHeight((int)m_thumbnailInfo.output_height);
    m_inputTmbSize.setWidth((int)m_thumbnailInfo.output_width);
    m_inputTmbPadSize.setHeight((int)m_thumbnailInfo.output_height);
    m_inputTmbPadSize.setWidth((int)m_thumbnailInfo.output_width);

    l_tmbScale_h = (float) m_inTmbPort->format.image.nFrameHeight /
      m_thumbnailInfo.output_height;
    l_tmbScale_w = (float) m_inTmbPort->format.image.nFrameWidth /
      m_thumbnailInfo.output_width;

    if (l_tmbScale_h > m_DmaMaxDownScale) {
      l_tmbScale_h = m_DmaMaxDownScale;
      m_inputTmbSize.setHeight(
        CEILING8((int)(m_inTmbPort->format.image.nFrameHeight / l_tmbScale_h)));
      m_inputTmbPadSize.setHeight(m_inputTmbSize.Height());
    }

    if (l_tmbScale_w > m_DmaMaxDownScale) {
      l_tmbScale_w = m_DmaMaxDownScale;
      m_inputTmbSize.setWidth(
        CEILING8((int)(m_inTmbPort->format.image.nFrameWidth / l_tmbScale_w)));
      m_inputTmbPadSize.setWidth(m_inputTmbSize.Width());
    }
  }

  QIDBG_HIGH("%s:%d] size %ux%u pad %ux%u out %ux%u subsampling %d",
    __func__, __LINE__,
    m_inputTmbSize.Width(), m_inputTmbSize.Height(),
    m_inputTmbPadSize.Width(), m_inputTmbPadSize.Height(),
    m_outputSize[1].Width(), m_outputSize[1].Height(),
    m_subsampling);

  m_thumbEncodeParams.setNumOfComponents(m_numOfComponents);

  if (m_UseDmaScale != true) {
    if ((m_thumbnailInfo.crop_info.nWidth != 0) &&
      (m_thumbnailInfo.crop_info.nHeight != 0) ) {
      l_UseEncoderCrop = true;
    }
  } else {
    //Dma can not upscale. Limit dma crop to output size
    if ((m_thumbnailInfo.crop_info.nWidth > m_thumbnailInfo.output_width) &&
      (m_thumbnailInfo.crop_info.nHeight > m_thumbnailInfo.output_height)) {
      m_UseDmaCrop = true;
    } else {
      l_UseEncoderCrop = true;
    }
  }

  if (l_UseEncoderCrop == true) {
    uint32_t left, right, top, bottom;

    left = (m_thumbnailInfo.crop_info.nLeft / l_tmbScale_w);
    right = (m_thumbnailInfo.crop_info.nWidth +
      m_thumbnailInfo.crop_info.nLeft) / l_tmbScale_w;
    top = m_thumbnailInfo.crop_info.nTop / l_tmbScale_h;
    bottom = (m_thumbnailInfo.crop_info.nHeight +
      m_thumbnailInfo.crop_info.nTop) / l_tmbScale_h;

    l_crop.setCrop(CEILING2((int)left), CEILING2((int)top),
      CEILING2((int)right), CEILING2((int)bottom));
    m_thumbEncodeParams.setCrop(l_crop);
    //Serialize two encoders if crop and upscale is used
    //Reason is (SID corruption on jpeg1 core)
    m_SerializeEncoders = true;
  }

  if ((0 < m_thumbnailInfo.quality) &&
    (OMX_MAX_THUMB_Q_FACTOR >= m_thumbnailInfo.quality)) {
    m_thumbEncodeParams.setQuality(m_thumbnailInfo.quality);
  } else {
    QIDBG_MED("%s:%d] Thumbnail quality factor invalid %u using default",
      __func__, __LINE__, m_thumbnailInfo.quality);
    m_thumbEncodeParams.setQuality(DEFAULT_THUMB_Q_FACTOR);
  }

  m_thumbEncodeParams.setOutputSize(m_outputSize[1]);
  m_thumbEncodeParams.setInputSize(m_inputTmbSize);
  m_thumbEncodeParams.setRestartInterval(0);
  m_thumbEncodeParams.setRotation((uint32_t)m_rotation.nRotation);
  m_thumbEncodeParams.setDefaultTables(m_thumbEncodeParams.Quality());
  m_thumbEncodeParams.setHiSpeed(m_JpegSpeedMode.speedMode ==
    QOMX_JPEG_SPEED_MODE_HIGH);
  return lret;
}

/*==============================================================================
* Function : startThumbnailEncode
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Start Thumbnail Encode
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::startThumbnailEncode(
    QImageCodecFactory::QCodecPrefType aPref)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lrc = 0;

  if (NULL == m_thumbEncoder) {
    if (m_thumbFormat == QI_MONOCHROME) {
      QIDBG_MED("%s:%d] Monochrome thumbnail format, switching to HW encoder",
        __func__, __LINE__);
      m_thumbEncoder = m_factory.CreateEncoder(QImageCodecFactory::HW_CODEC_ONLY,
        m_thumbEncodeParams);
    } else {
      m_thumbEncoder = m_factory.CreateEncoder(aPref, m_thumbEncodeParams);
    }
    if (m_thumbEncoder == NULL) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }
  }

  m_outThumbImage->SetFilledLen(0);

  lrc = m_thumbEncoder->SetOutputMode(QImageEncoderInterface::ENORMAL_OUTPUT);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  lrc = m_thumbEncoder->setEncodeParams(m_thumbEncodeParams);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  lrc = m_thumbEncoder->addInputImage(*m_inThumbImage);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  lrc = m_thumbEncoder->addOutputImage(*m_outThumbImage);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  lrc = m_thumbEncoder->addObserver(*this);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  m_thumbEncoding = OMX_TRUE;
#ifdef DUMP_THUMB
  m_inThumbImage->Dump("misc/camera/dma_thumb");
#endif
  QIDBG_HIGH("%s:%d] startThumbnailEncode()", __func__, __LINE__);
  lrc = m_thumbEncoder->Start(m_pEncoderThread);
  if (lrc ) {
    m_thumbEncoding = OMX_FALSE;
    QIDBG_ERROR("%s:%d] Thumbnail encoding failed to start",
      __func__, __LINE__);
    return OMX_ErrorUndefined;
  }
  ATRACE_INT("Camera:Jpeg:Thumb", 1);
  QIDBG_HIGH("%s:%d] Started Thumbnail encoding", __func__, __LINE__);
  return lret;

}

/*==============================================================================
* Function : preloadCodecLibs
* Parameters:
* Return Value : OMX_ERRORTYPE
* Description: pre-load the encoders
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::preloadCodecLibs()
{
  QIDBG_LOW("%s:%d] ", __func__, __LINE__);

  OMX_ERRORTYPE lret = configureEncodedata();
  if (lret != OMX_ErrorNone) {
    QIDBG_ERROR("%s:%d] Error in Encode configuration", __func__, __LINE__);
    return lret;
  }
  if (NULL == m_mainEncoder) {
    m_mainEncoder = m_factory.CreateEncoder(
      QImageCodecFactory::HW_CODEC_ONLY,
      m_mainEncodeParams, true);

    if (m_mainEncoder == NULL) {
      m_mainEncoder = m_factory.CreateEncoder(
        QImageCodecFactory::SW_CODEC_ONLY,
        m_mainEncodeParams);

      if (m_mainEncoder == NULL) {
        QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
        return OMX_ErrorInsufficientResources;
      } else {
        QIDBG_ERROR("%s:%d] Use SW_CODEC_ONLY flag true",
          __func__, __LINE__);
        m_UseSWEncoder = true;
      }
    }
  }

  /* TODO - possibly change to lazy allocation, move to after tmb info is set */
  uint32_t lTmbOutSz = TMB_OUT_MAX_SZ;

  m_jpegDma = m_dma_factory.CreateDMA(this, jpegDmaCallback);
  if (m_jpegDma == NULL) {
    QIDBG_ERROR("%s:%d] failed to get DMA engine",
      __func__, __LINE__);
    return OMX_ErrorNone;
  }

  if(JPEG_USE_NO_DMACPY) {
    m_jpegDmaCpy = NULL;
    QIDBG_MED("%s:%d] NO DMA CPY is used so don't create handle", __func__, __LINE__);
  } else {
    if (JPEG_USE_DMA_V4L2) {
      m_jpegDmaCpy = m_dma_factory.CreateDMA(this, jpegDmaCallback);
    } else {
      m_jpegDmaCpy = m_jpegDma;
    }

    if (m_jpegDmaCpy == NULL) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }
  }

  m_DmaState = JPEG_DMA_IDLE;
  m_jpegDmaOutBuf = QIONBuffer::New(lTmbOutSz, false);
  if (m_jpegDmaOutBuf == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorInsufficientResources;
  }

  m_DmaMaxDownScale = m_jpegDma->GetMaxDownScale();
  QIDBG_HIGH("%s:%d] Max Downscale factor %f", __func__, __LINE__,
    m_DmaMaxDownScale);

  /* Allocate thumbnail buffer */
  QIDBG_MED("%s:%d] lThumbSize %d", __func__, __LINE__, lTmbOutSz);
  mThumbBuffer = QIONBuffer::New(lTmbOutSz);
  if (mThumbBuffer == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorInsufficientResources;
  }

  m_outThumbImage = new QImage(mThumbBuffer->Addr(),
    mThumbBuffer->Length(), QI_BITSTREAM);
  if (m_outThumbImage == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  m_outThumbImage->setFd(mThumbBuffer->Fd());

  mSeqNo = 0;
  QIDBG_MED("%s:%d] X", __func__, __LINE__);
  return OMX_ErrorNone;
}

/*==============================================================================
* Function : encodeImage
* Parameters: OMX_BUFFERHEADERTYPE *a_inBuffer - Input Buffer passed during
* ETB, OMX_BUFFERHEADERTYPE *a_outBuffer - O/p buffer passed during FTB
* Return Value : OMX_ERRORTYPE
* Description: Start Image Encoding
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::encodeImage(
  OMX_BUFFERHEADERTYPE *a_inBuffer,
  OMX_BUFFERHEADERTYPE *a_inTmbBuffer,
  OMX_BUFFERHEADERTYPE *a_outBuffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  if (!a_inBuffer || !a_outBuffer) {
    QIDBG_ERROR("%s:%d] Bad parameter",  __func__, __LINE__);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s:%d] Invalid State",  __func__, __LINE__);
    return OMX_ErrorInvalidState;
  }

  pthread_mutex_lock(&m_abortlock);
  m_releaseFlag = OMX_FALSE;
  if (!m_abort_flag) {

    m_mainEncodingComplete = OMX_FALSE;
    if ((m_mainEncodeParams.Rotation() != (uint32_t)m_rotation.nRotation) &&
        (!m_mainEncodeParams.Rotation() || !m_rotation.nRotation)) {
      if (NULL != m_mainEncoder) {
        delete m_mainEncoder;
        m_mainEncoder = NULL;
      }
    }
    lret = configureEncodedata();
    if (lret != OMX_ErrorNone) {
      QIDBG_ERROR("%s:%d] Error in Encode configuration", __func__, __LINE__);
      goto error;
    }
    lret = configureBuffers(a_inBuffer, a_outBuffer);
    if (lret != OMX_ErrorNone) {
      QIDBG_ERROR("%s:%d] Error in Encode buffer configuration", __func__, __LINE__);
      goto error;
    }
    if ((m_thumbnailInfo.input_height != 0) &&
      (m_thumbnailInfo.input_width != 0)) {
      // Configure thumbnail buffer
      m_thumbEncodingComplete = OMX_FALSE;
      lret = configureTmbBuffer(a_inTmbBuffer);
      if (lret != OMX_ErrorNone) {
        QIDBG_ERROR("%s:%d] Error in Thumbnail bufffer configuration",
            __func__, __LINE__);
        goto error;
      }

      lret = configureThumbnailData();
      if (lret != OMX_ErrorNone) {
        QIDBG_ERROR("%s:%d] Error in Encode configuration", __func__, __LINE__);
        goto error;
      }

      if (m_UseDmaScale) {
        lret = startEncode();
        if (lret != OMX_ErrorNone) {
          QIDBG_ERROR("%s:%d] Error in Start Encode %d", __func__, __LINE__,
            lret);
          goto error;
        }

        lret = startDmaResize();
        if (lret != OMX_ErrorNone) {
          QIDBG_ERROR("%s:%d] Error in DMA resize %d", __func__, __LINE__,
            lret);
          goto error;
        }
        pthread_mutex_unlock(&m_abortlock);
        return OMX_ErrorNone;
      }

      /* Monochrome can only be encoded with HW encoder. Thus, switch to
         serial and use the HW for both thumbnail and main*/
      if (m_thumbFormat == QI_MONOCHROME) {
         m_encoding_mode = OMX_Serial_Encoding;
      }

      /* If Parallel encoding is enabled, encode both main image and thumbnail
         in parallel. Start main image first followed by thumbnail */
      if (m_encoding_mode == OMX_Parallel_Encoding) {
        lret = startEncode();
        if (lret != OMX_ErrorNone) {
          QIDBG_ERROR("%s:%d] Error in Start Encode", __func__, __LINE__);
          goto error;
        }
        lret = startThumbnailEncode(QImageCodecFactory::SW_CODEC_ONLY);
        if (lret != OMX_ErrorNone) {
          QIDBG_ERROR("%s:%d] Error in Starting thumbnail encode",
            __func__, __LINE__);
          goto error;
        }
      } else {
        /*If serial encoding start thumbnail first*/
        lret = startThumbnailEncode(QImageCodecFactory::SW_CODEC_ONLY);
        if (lret != OMX_ErrorNone) {
          QIDBG_ERROR("%s:%d] Error in Starting thumbnail encode",
            __func__, __LINE__);
          goto error;
        }
      }
    } else {
      /*If thumbnail is not present we call write exif from
        here with thumbnail as NULL*/
      if (NULL == m_memOps.get_memory) {
        lret = writeExifData(NULL, m_outputQIBuffer);
        if (lret != OMX_ErrorNone) {
          QIDBG_ERROR("%s:%d] Error in Exif Composer", __func__, __LINE__);
          goto error;
        }
        QIDBG_MED("%s:%d] Finished writing Exif", __func__, __LINE__);
      }
      lret = startEncode();
      if (lret != OMX_ErrorNone) {
        QIDBG_ERROR("%s:%d] Error in Start Encode", __func__, __LINE__);
        goto error;
      }
    }
  }

error:
  pthread_mutex_unlock(&m_abortlock);
  return lret;
}


/*==============================================================================
* Function : processMetadata
* Parameters: none
* Return Value : OMX_ERRORTYPE
* Description: Prepare and encrypt makernote data
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::processMetadata()
{
  OMX_ERRORTYPE lRet = OMX_ErrorNone;
  QExifCameraTuningParams lTuningParams;
  uint8_t *lParsedMeta = NULL;
  QCrypt *lCrypto = NULL;
  uint8_t *lEncData = NULL;
  int lCRet = 0;
  int lLen = 0;
  int lMsgLen = 0;

  if (NULL == m_Metadata.metadata) {
    QIDBG_ERROR("%s:%d] Metadata not present",
            __func__, __LINE__);
    return lRet;
  }

  lParsedMeta = new uint8_t[MAX_PARSED_METADATA_SIZE];
  if (NULL == lParsedMeta) {
    QIDBG_ERROR("%s:%d] Failed to allocate metadata buffer",
        __func__, __LINE__);
    lRet = OMX_ErrorInsufficientResources;
    goto cleanup;
  }

  lLen = lTuningParams.ExtractTuningInfo(m_Metadata.metadata, lParsedMeta,
    MAX_PARSED_METADATA_SIZE);
  if (lLen > MAX_PARSED_METADATA_SIZE) {
    QIDBG_ERROR("%s:%d] Parsed metadata output exceeds buffer",
        __func__, __LINE__);
    lRet = OMX_ErrorInsufficientResources;
    goto cleanup;
  }

  lCrypto = QCrypt::New();
  if (NULL == lCrypto) {
    QIDBG_ERROR("%s:%d] Failed to create qcrypt instance",
            __func__, __LINE__);
    lRet = OMX_ErrorInsufficientResources;
    goto cleanup;
  }

  lCRet = lCrypto->setEncKey((char*)m_MetadataEncKey.metaKey, m_MetadataEncKey.keyLen);
  if (QI_SUCCESS != lCRet) {
    QIDBG_ERROR("%s:%d] Failed to set encryption key",
        __func__, __LINE__);
    lRet = OMX_ErrorBadParameter;
    goto cleanup;
  }

  lMsgLen = lCrypto->setEncMsgLen(lLen);
  if (!lMsgLen) {
    QIDBG_ERROR("%s:%d] Failed to set message length",
        __func__, __LINE__);
    lRet = OMX_ErrorBadParameter;
    goto cleanup;
  }

  lEncData = new uint8_t[lMsgLen];
  if (NULL == lEncData) {
    QIDBG_ERROR("%s:%d] Failed to allocate encrypted data buffer",
        __func__, __LINE__);
    lRet = OMX_ErrorInsufficientResources;
    goto cleanup;
  }

  lMsgLen = lCrypto->encrypt(lParsedMeta, lEncData);
  if (!lMsgLen) {
    QIDBG_ERROR("%s:%d] Error during encryption",
        __func__, __LINE__);
    lRet = OMX_ErrorBadParameter;
    goto cleanup;
  }

  QIDBG_ERROR("%s:%d] Encrypted makernote of size %d",
             __func__, __LINE__, lMsgLen);

  exif_tag_entry_t lExifTag;
  lExifTag.type = EXIF_UNDEFINED;
  lExifTag.copy = 1;
  lExifTag.count = lMsgLen;
  lExifTag.data._undefined = lEncData;

  lCRet = exif_set_tag(m_exifInfoObj, EXIFTAGID_EXIF_MAKER_NOTE, &lExifTag);
  if (JPEGERR_SUCCESS != lCRet) {
    QIDBG_ERROR("%s:%d] Failed to set exif tag",
        __func__, __LINE__);
    lRet = OMX_ErrorBadParameter;
    goto cleanup;
  }
cleanup:
  if (NULL != lParsedMeta) {
    delete []lParsedMeta;
    lParsedMeta = NULL;
  }
  if (NULL != lCrypto) {
    delete lCrypto;
    lCrypto = NULL;
  }
  if (NULL != lEncData) {
    delete []lEncData;
    lEncData = NULL;
  }

  return lRet;
}


/*==============================================================================
* Function : releaseCodecLibs
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: release the instances of the encoder libs
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::releaseCodecLibs()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  QIDBG_MED("%s:%d] E", __func__, __LINE__);

  if (m_mainEncoder) {
    delete(m_mainEncoder);
    m_mainEncoder = NULL;
  }
  if (m_thumbEncoder) {
    delete(m_thumbEncoder);
    m_thumbEncoder = NULL;
  }

  if (m_jpegDmaOutBuf) {
    delete m_jpegDmaOutBuf;
    m_jpegDmaOutBuf = NULL;
  }
  if (mThumbBuffer) {
    delete mThumbBuffer;
    mThumbBuffer = NULL;
  }

  QIDBG_HIGH("%s:%d] X", __func__, __LINE__);
  return lret;
}

/*==============================================================================
* Function : releaseCurrentSession
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Release all buffers associated with the current snapshot.
* Encoding shold be stopped before calling this function. If not it will
* lead to a crash.
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::releaseCurrentSession()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  QIDBG_MED("%s:%d] E", __func__, __LINE__);

  QI_LOCK(&m_abortlock);
  m_releaseFlag = OMX_TRUE;
  m_thumbEncodingComplete = OMX_FALSE;
  m_mainEncodingComplete = OMX_FALSE;
  QI_UNLOCK(&m_abortlock);

  if (m_mainEncoder) {
    m_mainEncoder->ReleaseSession();
  }

  if (m_thumbEncoder) {
    m_thumbEncoder->ReleaseSession();
  }

  if (JPEG_USE_DMA_V4L2) {
    if (m_jpegDma) {
      m_jpegDma->ReleaseSession();
      delete m_jpegDma;
      m_jpegDma = NULL;
    }
    if (m_jpegDmaCpy) {
      m_jpegDmaCpy->ReleaseSession();
      delete m_jpegDmaCpy;
      m_jpegDmaCpy = NULL;
    }
  } else {
    if (m_jpegDma) {
      delete m_jpegDma;
      m_jpegDma = NULL;
      m_jpegDmaCpy = NULL;
    }
  }

  if (m_inThumbImage) {
    delete(m_inThumbImage);
    m_inThumbImage = NULL;
  }
  if (m_inputQIBuffer) {
    delete(m_inputQIBuffer);
    m_inputQIBuffer = NULL;
  }
  if (m_inputQTmbBuffer) {
    delete(m_inputQTmbBuffer);
    m_inputQTmbBuffer = NULL;
  }

  if (m_inputMainImage) {
    delete(m_inputMainImage);
    m_inputMainImage = NULL;
  }
  if (m_outputQIBuffer) {
    delete(m_outputQIBuffer);
    m_outputQIBuffer = NULL;
  }
  if (m_outputMainImage) {
    delete(m_outputMainImage);
    m_outputMainImage = NULL;
  }
  if (OMX_TRUE == mExifObjInitialized) {
    exif_destroy(&m_exifInfoObj);
    mExifObjInitialized = OMX_FALSE;
  }
  if (m_composer) {
    delete m_composer;
    m_composer = NULL;
  }
  if (m_mobicatComposer) {
    delete m_mobicatComposer;
    m_mobicatComposer = NULL;
  }
  if (m_InDmaImage) {
    delete m_InDmaImage;
    m_InDmaImage = NULL;
  }
  if (m_OutDmaImage) {
    delete m_OutDmaImage;
    m_OutDmaImage = NULL;
  }
  if (m_DmaMemCopyIn) {
    delete m_DmaMemCopyIn;
    m_DmaMemCopyIn = NULL;
  }
  if (m_DmaMemCopyOut) {
    delete m_DmaMemCopyOut;
    m_DmaMemCopyOut = NULL;
  }
  if (m_jpegOutMem) {
    delete m_jpegOutMem;
    m_jpegOutMem = NULL;
  }
  if (m_mainEncoder) {
    delete(m_mainEncoder);
    m_mainEncoder = NULL;
  }
  if (m_thumbEncoder) {
    delete(m_thumbEncoder);
    m_thumbEncoder = NULL;
  }

  QIDBG_MED("%s:%d] X", __func__, __LINE__);
  return lret;
}

/*==============================================================================
* Function : reserveJpegOutMem
* Parameters: Reserve jpeg output memory.
* Return Value : True if memory is reserved
* Description: Allocate and reserve jpeg output memory if output image is ready.
* NOTE: Caller of this function needs to take care of syncronization using
  m_abortlock. If Memory is already reserved it will return true.
==============================================================================*/
bool OMXJpegEncoderPipeline::reserveJpegOutMem(uint32_t app1Size)
{
  if (NULL == m_memOps.get_memory) {
    QIDBG_LOW("%s:%d Get memory callback is not set", __func__,  __LINE__);
    return false;
  }

  if ((m_outputMainImage == NULL) || (m_outputQIBuffer == NULL)) {
    QIDBG_ERROR("%s:%d Output image is not set", __func__,  __LINE__);
    return false;
  }

  if (OMX_FALSE == m_mainEncodingComplete) {
    QIDBG_MED("%s:%d Main image encoding is not completed", __func__,  __LINE__);
    return false;
  }

  if (!m_outputMainImage->FilledLen()) {
    QIDBG_MED("%s:%d Main image length is 0", __func__,  __LINE__);
    return false;
  }

  if (m_jpegOutMem == NULL) {
    QISize lDmaMemcpySize;
    uint32_t lIonOutSize;
    omx_jpeg_ouput_buf_t *lJpegOut =
      (omx_jpeg_ouput_buf_t *)m_outputQIBuffer->Addr();;

    //Allocate ion output buffer based on Dma calculated size
    getDmaMemcpySize(m_outputMainImage->FilledLen(), lDmaMemcpySize);
    lIonOutSize = getEstimatedExifSize() + lDmaMemcpySize.Length();

    ATRACE_BEGIN("Camera:Jpeg:GetOutputBuffer");
    m_jpegOutMem = QIONBuffer::New(lIonOutSize, true, false);
    if (m_jpegOutMem == NULL) {
      QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      ATRACE_END();
      goto error;
    }
    lJpegOut->size = app1Size + lDmaMemcpySize.Length();
    QIDBG_MED("%s:%d Allocating full bit stream buffer size = %d, App1 size = %d",
      __func__, __LINE__, m_outputMainImage->FilledLen(), app1Size);

    lJpegOut->fd = m_jpegOutMem->Fd();

    m_memOps.get_memory(lJpegOut, m_memOps.psession);
    if(!lJpegOut->vaddr) {
      QIDBG_ERROR("%s:%d get_memory failed", __func__,  __LINE__);
      ATRACE_END();
      goto error;
    }
    ATRACE_END();
  }

  return true;

error:
  return false;
}

/*==============================================================================
* Function : aBufferLenght
* Parameters: aLenghtBytes buffer lenght need to be copied in bytes.
*  aDmaMemCopySize - output size width and height need to be used during
*  Dma memcopy.
* Return Value : OMX_ERRORTYPE
* Description: Calculate Dma memcopy width and height based in buffer lenght
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::getDmaMemcpySize(uint32_t aLenghtBytes,
  QISize &aDmaSize)
{
  uint32_t lDmaWidth, lDmaHeight;
  uint32_t offset;

  lDmaWidth = (uint32_t)sqrt(aLenghtBytes);
  lDmaWidth = QI_ALIGN(lDmaWidth, DMA_MEMCPY_WIDTH_ALIGN);

  lDmaHeight = (aLenghtBytes + lDmaWidth - 1) / lDmaWidth;
  lDmaHeight = QI_ALIGN(lDmaHeight, DMA_MEMCPY_HEIGHT_ALIGN);

  aDmaSize.setHeight((int)lDmaHeight);
  aDmaSize.setWidth((int)lDmaWidth);

  QIDBG_MED("%s:%d] DMA MemCopy Lenght %d calculated size Size %dx%d",
    __func__, __LINE__, aLenghtBytes, lDmaWidth, lDmaHeight);

  return OMX_ErrorNone;
}

/*==============================================================================
* Function : startSwMemcpy
* Parameters: aInImg - Input image, aOutBuf - Output image.
* Return Value : OMX_ERRORTYPE
* Description: Start SW memcpy to output buffer.
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::startSwMemcpy(QImage &aInImg,
  QIBuffer &aOutBuf)
{
  int lQIret;

  ATRACE_BEGIN("Camera:Jpeg:SWCopy");
#ifndef TARGET_ION_ABI_VERSION
  //Invalidate old cache lines before memcpy of output buffer
  if (aInImg.Fd() >= 0) {
    QIONBuffer::DoCacheOps(aInImg.Fd(), aInImg.FilledLen(),
      QIONBuffer::CACHE_INVALIDATE);
  }
#endif

  memcpy(aOutBuf.Addr() + aOutBuf.FilledLen(),
    aInImg.BaseAddr(), aInImg.FilledLen());
  ATRACE_END();

  //Main image is ready complete
  lQIret = ReleaseMainImage();
  if (lQIret != QI_SUCCESS) {
    QIDBG_ERROR("%s:%d ", __func__,  __LINE__);
    return OMX_ErrorUndefined;
  }

  return OMX_ErrorNone;
}

/*==============================================================================
* Function : startDmaMemcpy
* Parameters: aInImg - Input image, aOutBuf - Output image.
* Return Value : OMX_ERRORTYPE
* Description: Start jpegDma memcpy to output buffer.
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::startDmaMemcpy(QImage &aInImg,
  QIBuffer &aOutBuf, QISize *aDmaSize)
{
  QISize lDmaInSize, lDmaInPadSize;
  QIFormat format = QI_MONOCHROME;
  QISubsampling subsampling = QI_H1V1;
  int32_t lrc;
  OMX_ERRORTYPE lOMXErr;
  uint32_t offset;
  uint32_t length;
  QIPlane *l_p_plane;

  if (m_jpegDmaCpy == NULL || m_DmaState != JPEG_DMA_IDLE) {
    QIDBG_ERROR("%s:%d] JpegDma not ready %p %d", __func__, __LINE__,
      m_jpegDmaCpy, m_DmaState);
    return OMX_ErrorInsufficientResources;
  }

  //Get Dma input sized based on memcopy lenght
  if (aDmaSize == NULL) {
    getDmaMemcpySize(aInImg.FilledLen(), lDmaInSize);
  } else {
    lDmaInSize = *aDmaSize;
  }
  lDmaInPadSize = lDmaInSize;

  m_DmaMemCopyIn = new QImage(lDmaInSize, subsampling, format);
  if (m_DmaMemCopyIn == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    lOMXErr = OMX_ErrorInsufficientResources;
    goto error;
  }

  lrc = m_DmaMemCopyIn->setDefaultPlanes(1, aInImg.BaseAddr(), aInImg.Fd());
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    lOMXErr =  OMX_ErrorUndefined;
    goto error;
  }

  m_DmaMemCopyOut = new QImage(lDmaInSize, subsampling, format);
  if (m_DmaMemCopyOut == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    lOMXErr = OMX_ErrorInsufficientResources;
    goto error;
  }

  offset = aOutBuf.FilledLen();
  QIDBG_HIGH("%s:%d] Filled length of DMA cpy out buff %d ",
    __func__, __LINE__, offset);

  lrc = m_DmaMemCopyOut->setDefaultPlanes(1, aOutBuf.Addr(),
    aOutBuf.Fd(), &offset, &offset);
  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    lOMXErr = OMX_ErrorUndefined;
    goto error;
  }

  l_p_plane = m_DmaMemCopyOut->getPlane(QIPlane::PLANE_Y);
  if (l_p_plane == NULL) {
    QIDBG_ERROR("%s:%d] failed to get plane", __func__, __LINE__);
    lOMXErr = OMX_ErrorInsufficientResources;
    goto error;
  }

  length = l_p_plane->Length();
  QIDBG_HIGH("%s:%d] length to cpy %d ", __func__, __LINE__, length);
  QIDBG_HIGH("%s:%d] buff len %d", __func__, __LINE__, aOutBuf.Length());

  if ((offset + length) > aOutBuf.Length()) {
    QIDBG_ERROR("%s:%d] insufficient buffer length, req len %d, buff len %d",
      __func__, __LINE__, (offset + length), aOutBuf.Length());
    lOMXErr = OMX_ErrorInsufficientResources;
    goto error;
  }

  // Before start new session release previous one
  m_jpegDmaCpy->ReleaseSession();

  ATRACE_INT("Camera:Jpeg:DMACopy", 1);
  lrc = m_jpegDmaCpy->Start(*m_DmaMemCopyIn, *m_DmaMemCopyOut, false);
  if (QI_SUCCESS != lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    ATRACE_END();
    lOMXErr = OMX_ErrorUndefined;
    goto error;
  }
  m_DmaState = JPEG_DMA_MEMCPY;

  return OMX_ErrorNone;

error:

  ATRACE_INT("Camera:Jpeg:DMACopy", 0);
  return lOMXErr;
}

/*==============================================================================
* Function : startDmaResize
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Start resize of the thumbnail image with the HW DMA engine
==============================================================================*/
OMX_ERRORTYPE OMXJpegEncoderPipeline::startDmaResize()
{
  QISize lDmaInSize, lDmaInPadSize;
  uint32_t lDmaWidth, lDmaHeight;
  QOMX_YUV_FRAME_INFO *lbufferOffset = &m_thumbnailInfo.tmbOffset;
  int lrc;

  if ((m_jpegDma == NULL) || (m_DmaState != JPEG_DMA_IDLE)) {
    QIDBG_ERROR("%s:%d] JpegDma not ready %p %d", __func__, __LINE__,
      m_jpegDma, m_DmaState);
    return OMX_ErrorInsufficientResources;
  }

  if (!m_inTmbPort->bEnabled) {
    lbufferOffset = &m_imageBufferOffset;
    QIDBG_ERROR("%s:%d] TMB PORT IS NOT ENABLED", __func__, __LINE__);
  }

  // Set the offset for each plane
  uint32_t lOffset[OMX_MAX_NUM_PLANES] = {lbufferOffset->yOffset,
    lbufferOffset->cbcrOffset[0] , lbufferOffset->cbcrOffset[1]};

  QIDBG_MED("%s:%d] DMA in lOffset: %d %d %d", __func__, __LINE__,
    lOffset[0], lOffset[1], lOffset[2]);

  // Set the physical offset for each plane
  uint32_t lPhyOffset[QI_MAX_PLANES] = {0,
    lbufferOffset->cbcrStartOffset[0],
    lbufferOffset->cbcrStartOffset[1]};

  QIDBG_MED("%s:%d] DMA in lPhyOffset: %d %d %d", __func__, __LINE__,
    lPhyOffset[0], lPhyOffset[1], lPhyOffset[2]);

  lDmaWidth = m_inTmbPort->format.image.nFrameWidth;
  lDmaHeight =  m_inTmbPort->format.image.nFrameHeight;

  lDmaInSize.setHeight((int)lDmaHeight);
  lDmaInSize.setWidth((int)lDmaWidth);
  lDmaInPadSize.setHeight((int)m_inTmbPort->format.image.nSliceHeight);
  lDmaInPadSize.setWidth((int)m_inTmbPort->format.image.nStride);

  m_InDmaImage = new QImage(lDmaInPadSize, m_thumbSubsampling, m_thumbFormat,
    lDmaInSize);

  if (m_InDmaImage == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorInsufficientResources;
  }

  lrc = m_InDmaImage->setDefaultPlanes(m_numOfPlanes, m_inputQTmbBuffer->Addr(),
    m_inputQTmbBuffer->Fd(), lOffset, lPhyOffset);

  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  m_OutDmaImage = new QImage(m_inputTmbPadSize, m_thumbSubsampling, m_thumbFormat,
    m_inputTmbSize);

  if (m_OutDmaImage == NULL) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorInsufficientResources;
  }

  lrc = m_OutDmaImage->setDefaultPlanes(m_numOfPlanes, m_jpegDmaOutBuf->Addr(),
    m_jpegDmaOutBuf->Fd());

  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  m_inThumbImage = new QImage(m_inputTmbPadSize, m_thumbSubsampling, m_thumbFormat,
    m_inputTmbSize);

  lrc = m_inThumbImage->setDefaultPlanes(m_numOfPlanes,
    m_jpegDmaOutBuf->Addr(),
    m_jpegDmaOutBuf->Fd());

  if (lrc) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  QIDBG_HIGH("%s:%d] input %ux%u out %ux%u ", __func__, __LINE__,
    m_InDmaImage->Size().Width(), m_InDmaImage->Size().Height(),
    m_OutDmaImage->Size().Width(), m_OutDmaImage->Size().Height());

  if (m_UseDmaCrop == true) {
    uint32_t left, right, top, bottom;

    left = m_thumbnailInfo.crop_info.nLeft;
    right = left + m_thumbnailInfo.crop_info.nWidth;
    top = m_thumbnailInfo.crop_info.nTop;
    bottom = top + m_thumbnailInfo.crop_info.nHeight;
    QICrop aInCrop(left, top, right, bottom);
    lrc = m_jpegDma->Start(*m_InDmaImage, *m_OutDmaImage, aInCrop, false);
  } else {
    lrc = m_jpegDma->Start(*m_InDmaImage, *m_OutDmaImage, false);
  }

  if (QI_SUCCESS != lrc) {
    QIDBG_ERROR("%s:%d] DMA start failed", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }
  m_DmaState = JPEG_DMA_DOWNSCALE;

  return OMX_ErrorNone;
}

/*==============================================================================
* Function : jpegDmaCallback
* Parameters: priv - client private data, Img - output image, stat - transfer
* status
* Return Value : None
* Description: Callback function to signal transfer done
==============================================================================*/
void OMXJpegEncoderPipeline::jpegDmaCallback(void *priv,
  QImage *Img __unused,
  int status)
{
  OMX_ERRORTYPE lOMXrc = OMX_ErrorNone;
  int lIQrc;
  OMXJpegEncoderPipeline *lInst = (OMXJpegEncoderPipeline*)priv;

  QImageCodecFactory::QCodecPrefType lTEncPref =
    QImageCodecFactory::SW_CODEC_ONLY;

  if (status != QI_SUCCESS) {
    if (lInst->m_DmaState == JPEG_DMA_DOWNSCALE) {
      lInst->EncodeError(ERROR_THUMBNAIL_DROPPED);
    } else {
      lInst->EncodeError(ERROR_GENERAL);
    }
    return;
  }

  QIDBG_MED("%s:%d]  E", __func__, __LINE__);
  QI_LOCK(&lInst->m_abortlock);
  if (!lInst->m_abort_flag && (OMX_FALSE == lInst->m_releaseFlag)) {
    QIDBG_MED("%s:%d] DMA state %d", __func__, __LINE__,lInst->m_DmaState);
    switch (lInst->m_DmaState) {
    case JPEG_DMA_DOWNSCALE:
      if (lInst->m_ForceHWThumbnail && !lInst->m_mainEncodingComplete) {
        QIDBG_HIGH("%s:%d] JPEG_DMA_DOWNSCALE defer %d",
          __func__, __LINE__, lInst->m_mainEncodingComplete);
      } else {
        if (lInst->m_ForceHWThumbnail ||
          (lInst->m_mainEncodingComplete && !lInst->m_UseSWEncoder)) {
          // Avoid parallel encoding start thumb only if main image is done
          lTEncPref = QImageCodecFactory::HW_CODEC_PREF;
        }

        QIDBG_HIGH("%s:%d] JPEG_DMA_DOWNSCALE preference %d",
          __func__, __LINE__, lTEncPref);
        // use SW encoder if DMA downscale is completed first
        lOMXrc = lInst->startThumbnailEncode(lTEncPref);
        if (lOMXrc != OMX_ErrorNone) {
          QIDBG_ERROR("%s:%d] startThumbnailEncode fail %d",
            __func__, __LINE__, lOMXrc);
        }
      }
      break;
    case JPEG_DMA_MEMCPY:
      QIDBG_HIGH("%s:%d] JPEG_DMA_MEMCPY", __func__, __LINE__);
      // If DMA Memcpy is done we can release main image

      ATRACE_INT("Camera:Jpeg:DMACopy", 0);
      ATRACE_BEGIN("Camera:Jpeg:Release");
      lIQrc = lInst->ReleaseMainImage();
      ATRACE_END();

      if (lIQrc != QI_SUCCESS) {
        QIDBG_ERROR("%s:%d] ReleaseMainImage fail", __func__, __LINE__);
      }
      break;
    case JPEG_DMA_IDLE:
    default:
      QIDBG_ERROR("%s:%d] Invalid Dma state %d", __func__, __LINE__,
        lInst->m_DmaState);
      break;
    }
    lInst->m_DmaState = JPEG_DMA_IDLE;
  }
  QI_UNLOCK(&lInst->m_abortlock);
}
