/*******************************************************************************
* Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*******************************************************************************/

#include "OMXImageEncoder.h"
#include "QExifComposerParams.h"
#include "QExifComposer.h"
#include "QImageEncoderObserver.h"
#include "QIHeapBuffer.h"
#include "QExifCameraTuningParams.h"
#include "QCrypt.h"
#include "QImageDMAFactory.h"

/*===========================================================================
 * Class: OMXJpegEncoderPipeline
 *
 * Description: This class represents the core OMX Jpeg encoder component
 *
 * Notes: none
 *==========================================================================*/
class OMXJpegEncoderPipeline: public OMXImageEncoder, QImageEncoderObserver,
  QImageWriterObserver
{

public:
  /** OMXImageEncoder
   *
   * Contsructor
   **/
  OMXJpegEncoderPipeline();

  /** ~OMXImageEncoder
   *
   * Destructor
   **/
  ~OMXJpegEncoderPipeline();

protected:

  /** encodeImage
   *  @a_inBuffer: input buffer header
   *  @a_outBuffer: output buffer header
   *
   *  Start Image Encoding
   **/
  virtual OMX_ERRORTYPE encodeImage(OMX_BUFFERHEADERTYPE *a_inBuffer,
    OMX_BUFFERHEADERTYPE *a_inTmbBuffer,
    OMX_BUFFERHEADERTYPE *a_outBuffer);

  /** configureBuffers
   *  @a_inBuffer: input buffer header
   *  @a_outBuffer: output buffer header
   *
   *  Configure the input and output buffers
   **/
  virtual OMX_ERRORTYPE configureBuffers(OMX_BUFFERHEADERTYPE *a_inBuffer,
    OMX_BUFFERHEADERTYPE *a_outBuffer);

  /** configureEncodedata
   *
   *  Configure the encode parmeters
   **/
  virtual OMX_ERRORTYPE configureEncodedata();

  /** processMetadata
   *
   *  Process makernote data
   **/
  virtual OMX_ERRORTYPE processMetadata();

  /*==============================================================================
   * Function : configureTmbBuffer
   * Parameters: a_inBuffer, a_outBuffer
   * Return Value : OMX_ERRORTYPE
   * Description: Configure input thumbnail buffer
   ==============================================================================*/
  OMX_ERRORTYPE configureTmbBuffer(
      OMX_BUFFERHEADERTYPE *a_inTmbBuffer);

  /** writeExifData
   *  @aThumbnail: The thumbnail buffer after encoding. Null
   *             when thumbnail is not encoded
   *
   *  Add the exif data and encoded thumbnail to the o/p buffer
   **/
  OMX_ERRORTYPE writeExifData(QImage *aThumbnail = NULL, QIBuffer *aOutBuffer = NULL);

  /** startEncode
   *
   *  Get the encoder from the factory and start encoding
   **/
  OMX_ERRORTYPE startEncode();

  /** ReleaseMainImage
   *
   * Send back main image to omx.
  **/
  int ReleaseMainImage();

  /** CompleteMainImage
   *
   * When encoding thumbnail and main image in parallel, call
   * this function to release all buffers associated with the
   * current snapshot.
  **/
  int CompleteMainImage();

  /** configureThumbnailData
   *
   *  Configure the encode parameters for thumbnail
   **/
  OMX_ERRORTYPE configureThumbnailData();

  /** startThumbnailEncode
   *
   *  Start Thumbnail Encode
   **/
  OMX_ERRORTYPE startThumbnailEncode(QImageCodecFactory::QCodecPrefType aPref);

  /** thumbnailEncodePending
   *
   *  Thumbnail encode is pending
   **/
  bool thumbnailEncodePending();

  /** WriteFragmentDone
   *  @aBuffer: output buffer
   *
   *  Callback function from ExifComposer for piecewise composing
   **/
  void WriteFragmentDone(QIBuffer &aBuffer);

  /** WriteFragmentDone
   *  @aBuffer: output buffer
   *
   *  Callback function from ExifComposer to indicate the
   *  completion of exif composition
   **/
  void WriteComplete(QIBuffer &aBuffer);

  /** WriteError
   *  aErrorType: error type
   *
   *  Callback function from ExifComposer to indicate error
   **/
  void WriteError(ErrorType aErrorType);

  /** OutputFragment
   *  @aBuffer: output buffer
   *
   *  Callback function from encoder tramework for piecewise
   *  encoding
   **/
  int OutputFragment(QIBuffer &aBuffer);

  /** EncodeError
   *  aErrorType: error type
   *
   *  Callback function from encoder tramework to indicate error
   **/
  int EncodeError(EncodeErrorType aErrorType);

  /** EncodeComplete
   *
   *  This function is called from the JPEG component framework
   *  when encoding is complete
   **/
  int EncodeComplete(QImage *aOutputImage);

  /** releaseCurrentSession
   *
   *  Release all buffers associated with the current snapshot.
   *  Encoding shold be stopped before calling this function. If
   *  not it will lead to a crash.
   **/
  OMX_ERRORTYPE releaseCurrentSession();

  /** releaseCodecLibs
   *
   *  Release the encoder instances
   **/
  OMX_ERRORTYPE releaseCodecLibs();

  /** preloadCodecLibs
   *
   *  pre-load the encoders
   **/
  OMX_ERRORTYPE preloadCodecLibs();

  /** getRestartInterval
   *
   *  gets the restart Interval value. Hybrid Q6/SW based encoder
   *  requires restart marker to be inserted at every MCUrow.
   *  On the contrary, HW encoder does not use restart interval.
   *  JPEG_USE_QDSP6_ENCODER is used to differentiate.
   **/
  uint32_t getRestartInterval(QIEncodeParams& m_params);

  /** startDmaResize
   *
   *  Start resize of the thumbnail image with the HW DMA engine
   **/
  OMX_ERRORTYPE startDmaResize();

  /** reserveJpegOutMem
   *
   *  Reserve jpeg output memory if main image encoding is completed.
   **/
  bool reserveJpegOutMem(uint32_t app1Size);

  /** GetDmaMemcpySize
   *
   *  Get dma image size required for dma memcpy.
   **/
  OMX_ERRORTYPE getDmaMemcpySize(uint32_t aLenghtBytes, QISize &aDmaSize);

  /** startDmaResize
   *
   *  Start Dma memcpy of jpeg image to output buffer
   **/
  OMX_ERRORTYPE startDmaMemcpy(QImage &aInImg, QIBuffer &aOutBuf,
    QISize *aDmaSize = NULL);

  /** OMX_ERRORTYPE
   *
   *  Start sw memcpy of jpeg image to output buffer
   **/
  OMX_ERRORTYPE startSwMemcpy(QImage &aInImg, QIBuffer &aOutBuf);

  /** startDmaResize
   *
   *  Callback function to signal transfer done
   *  @priv - client private data
   *  @Img - output image
   *  @stat - transfer status
   **/
  static void jpegDmaCallback(void *priv,  QImage *Img, int stat);

  /** m_dma_factory
   *
   *  HW Jpeg dma factory
   **/
  QImageDMAFactory m_dma_factory;

  /** m_jpegDma
   *
   *  HW Jpeg dma engine handle
   **/
  QImageDMAInterface *m_jpegDma;

  /** m_jpegDmaCpy
   *
   *  HW Jpeg dma engine handle
   **/
  QImageDMAInterface *m_jpegDmaCpy;

  /** m_jpegDmaOutBuf
   *
   *  Jpeg dma output buffer
   **/
  QIBuffer *m_jpegDmaOutBuf;

  /** m_jpegOutMem
   *
   *  Jpeg out memory.
   **/
  QIONBuffer *m_jpegOutMem;

  /** m_InDmaImage
   *
   *  Jpeg dma input image
   **/
  QImage *m_InDmaImage;

  /** m_OutDmaImage
   *
   *  Jpeg dma output image
   **/
  QImage *m_OutDmaImage;

  /** m_DmaMemcpyIn
   *
   *  Jpeg dma memcpy input
   **/
  QImage *m_DmaMemCopyIn;

  /** m_DmaMemcpyOut
   *
   *  Jpeg dma memcpy output
   **/
  QImage *m_DmaMemCopyOut;

  /** m_UseDmaScale
   *
   *  Use the dma for thumbnail image scaling
   **/
  bool m_UseDmaScale;

  /** m_UseDmaCrop
   *
   *  Use the dma crop for thumbnail image scaling
   **/
  bool m_UseDmaCrop;

  /** m_SerializeEncoders
   *
   *  Flag indicating that two encoders need to be serialized
   **/
  bool m_SerializeEncoders;

  /** m_DmaState
   *
   *  Jpeg dma state.
   **/
  enum {
    JPEG_DMA_IDLE,
    JPEG_DMA_DOWNSCALE,
    JPEG_DMA_MEMCPY,
  } m_DmaState;

  /** m_UseSWEncoder
   *
   *  Flag indicating to use software encoder
   **/
  bool m_UseSWEncoder;

  /** m_pEncoderThread
   *
   *  Jpeg encoder QIThread to be passed to QImageEncoder
   **/
  QIThread *m_pEncoderThread;

  /** m_ForceHWThumbnail
   *
   *  Flag indicating to use only HW for thumbnail
   **/
  bool m_ForceHWThumbnail;

  /** m_DmaMaxDownScale
   *
   *  Max DMA downscale factor
   **/
  float m_DmaMaxDownScale;
};
