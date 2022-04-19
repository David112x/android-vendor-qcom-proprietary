#ifndef DUALCAMERADDM_WRAPPER_H
#define DUALCAMERADDM_WRAPPER_H

/**=============================================================================
Copyright (c) 2016-2018 Qualcomm Technologies, Inc.  All rights reserved.
Export of this technology or software is regulated by the U.S.
Government. Diversion contrary to U.S. law prohibited.
All ideas, data and information contained in or disclosed by
this document are confidential and proprietary information of
Qualcomm Technologies Incorporated and all rights therein are expressly reserved.
By accepting this material the recipient agrees that this material
and the information contained therein are held in confidence and in
trust and will not be used, copied, reproduced in whole or in part,
nor its contents revealed in any manner to others without the express
written permission of Qualcomm Technologies Incorporated.
=============================================================================**/


//==============================================================================
// Included modules
//==============================================================================
#include <stdint.h>
#include <limits>
#include <string>
#include <algorithm>
//==============================================================================
// MACROS
//==============================================================================
#ifdef _MSC_VER
#define CP_DLL_PUBLIC __declspec(dllexport)
#else
#define CP_DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif

//==============================================================================
//                               Declarations
//                             External functions
//==============================================================================
class DualCameraEffects;

CP_DLL_PUBLIC bool dualcameraddm_qSv3fcF7Tp( int, int, int&, int& );
CP_DLL_PUBLIC void dualcameraddm_k5DFT9dbFc( const uint8_t*, const uint8_t*, int,
                                             int, int, int, const uint8_t*,
                                             const uint8_t*, int, int, int,
                                             int, uint8_t*, int, int,
                                             int, int&, int&, int&, int&,
                                             const char*, const char*, const char*,
                                             const float, int, int, int&, const char*& );

CP_DLL_PUBLIC bool dualcameraddm_sKy6qu3UbY( const uint8_t*, int, int, int,
                                             uint8_t*, int, int, int );

//==============================================================================
//                            Namespace qrcp
//==============================================================================

namespace qrcp
{
namespace internal
// Functions within internal namespace are meant for internal usecase only
// They shouldn't be called outside this header
{
//==============================================================================
// UTILITY FUNCTIONS
//==============================================================================
static inline bool
isImageDimensionsValid( int width, int height, int stride, int channels )
{
   if( width <= 0 || width > std::numeric_limits<int32_t>::max() )
   {
      return false;
   }

   if( height <= 0 || height > std::numeric_limits<int32_t>::max() )
   {
      return false;
   }

   if( stride < width * channels || stride > std::numeric_limits<int32_t>::max() )
   {
      return false;
   }

   return true;
}

static inline bool
isInputImageDataValid( const uint8_t* imgPtr, int width, int height,
                       int stride, int numChannels )
{
   // Validate image pointer
   // Image pointer should be non-NULL
   if( imgPtr == NULL )
   {
      return false;
   }

   // Validate width, height, and stride are correct
   if( !isImageDimensionsValid( width, height, stride, numChannels ) )
   {
      return false;
   }

   //Read data from image buffer
   //Should not cause memory access violation
   volatile uint8_t tmp;
   tmp = imgPtr[0];
   tmp = imgPtr[stride * height - 1];

   return true;
}

static inline bool
isOutputImageDataValid( uint8_t* imgPtr, int width, int height,
                        int stride, int numChannels )
{
   // Validate image pointer
   // Image pointer should be non-NULL
   if( imgPtr == NULL )
   {
      return false;
   }

   // Validate width, height, and stride are correct
   if( !isImageDimensionsValid( width, height, stride, numChannels ) )
   {
      return false;
   }

   //Write data to image buffer
   //Should not cause memory access violation
   volatile uint8_t tmp;
   tmp = imgPtr[0];
   imgPtr[0] = tmp;

   tmp = imgPtr[stride * height - 1];
   imgPtr[stride * height - 1] = tmp;

   return true;
}
} // namespace internal

class CP_DLL_PUBLIC DDMWrapperStatus
{
public:
   enum ErrorCode
   {
      OK = 0,                       // Everything is Ok. No error
      INVALID_ARGUMENT = 1,         // Function received an invalid argument
      INIT_FAIL = 2,                // Initialization failed
      INTERNAL_ERROR = 3,           // Internal error occured in implementation
      BAD_SEGMENTATION = 4,         // Object at user selected point is not segmentable
   };

   ///-----------------------------------------------------------------------------
   /// @brief
   ///  Constructor
   ///-----------------------------------------------------------------------------
   DDMWrapperStatus() : _code( ErrorCode::INIT_FAIL ), _errMsg( "Default DDMWrapperStatus initialization" ) {}

   ///-----------------------------------------------------------------------------
   /// @brief
   ///  Constructor
   ///-----------------------------------------------------------------------------
   DDMWrapperStatus( const ErrorCode& code, const std::string& errMsg ) : _code( code ), _errMsg( errMsg ) {}

   ///-----------------------------------------------------------------------------
   /// @brief
   ///   returns true if ErrorCode is OK, false otherwise
   ///-----------------------------------------------------------------------------
   bool ok() const
   {
      return _code == ErrorCode::OK;
   }

   ///-----------------------------------------------------------------------------
   /// @brief
   ///   Get error code
   ///-----------------------------------------------------------------------------
   ErrorCode getErrorCode() const
   {
      return _code;
   }

   ///-----------------------------------------------------------------------------
   /// @brief
   ///   Get explanation for error code
   ///-----------------------------------------------------------------------------
   const std::string& getErrorMessage() const
   {
      return _errMsg;
   }

private:
   const ErrorCode _code;
   const std::string _errMsg;
};

//==============================================================================
// API FUNCTIONS
//==============================================================================

//------------------------------------------------------------------------------
/// @brief
///   Get depth map output size
///   This API should be called first to get size of depth map so that
///   correct size output buffer can be pre-allocated
///
/// @param primaryWidth
///   Width of primary image
///
/// @param primaryHeight
///   Height of primary image
///
/// @param depthMapWidth
///   Output depth map width
///
/// @param depthMapHeight
///   Output depth map height
///
/// @return
///   True if successful; false otherwise
//------------------------------------------------------------------------------
static inline
bool getDepthMapSize( int primaryWidth, int primaryHeight, int& depthMapWidth, int& depthMapHeight )
{
   return dualcameraddm_qSv3fcF7Tp( primaryWidth, primaryHeight, depthMapWidth, depthMapHeight );
}

//------------------------------------------------------------------------------
/// @brief
///   This enum defines different depth map modes
///   You can sacrifice depth map quality for speed
///
///   ENHANCED_MODE: Computes best quality depth map in reasonable time on premium tier chipsets
///                    NOTE: This is the default option
///
///   NORMAL_MODE: Computes good quality depth map in reasonable time on mid/low tier chipsets
//------------------------------------------------------------------------------
enum DepthMapMode
{
   ENHANCED_MODE = 1,
   NORMAL_MODE = 2
};

//------------------------------------------------------------------------------
/// @brief
///   This enum defines combinations of sensor configurations supported by this library
///
///   SYMMETRIC_BAYER_MONO: Primary is Bayer symmetric lens whereas
///                         auxiliary is Monochrome symmetric lens
///
///   STANDARD_WIDE_BAYER_BAYER: Primary is Bayer standard lens whereas
///                              auxiliary is Bayer Wide angle lens
///
///   SYMMETRIC_BAYER_BAYER: Primary sensor is Bayer standard (normal) lens whereas
///                              auxiliary sensor is Bayer standard (normal) lens
///
///   TELE_WIDE_BAYER_BAYER: Primary sensor is Bayer Tele lens whereas
///                              auxiliary sensor is Bayer Wide angle lens
//------------------------------------------------------------------------------
enum SensorConfiguration
{
   SYMMETRIC_BAYER_MONO = 0,
   STANDARD_WIDE_BAYER_BAYER = 1,
   SYMMETRIC_BAYER_BAYER = 2,
   TELE_WIDE_BAYER_BAYER = 3,
   SENSOR_CONFIGURATION_SIZE = 4
};

//------------------------------------------------------------------------------
/// @brief
///   Generate depth map from primary and auxiliary NV21 images
///
/// @param primaryY
///   Single channel 8 bit luma channel of primary image
///
/// @param primaryVU
///   Two channel 8 bit chroma channels of primary image
///
/// @param primaryStrideY
///   Stride of luma primary image
///
/// @param primaryStrideVU
///   Stride of chroma primary image
///
/// @param auxiliaryY
///   Single channel 8 bit luma channel of auxiliary image
///
/// @param auxiliaryVU
///   Two channel 8 bit chroma channels of auxiliary image
///
/// @param auxiliaryStrideY
///   Stride of luma auxiliary image
///
/// @param auxiliaryStrideVU
///   Stride of chroma auxiliary image
///
/// @param dst
///   Output depth map
///   Pre-allocated dst buffer should be single channel 8 bit and of correct depth map size
///   Get depth map size by calling getDepthMapSize() API before calling this function
///
/// @param dstStride
///   Stride of dst buffer
///
/// @param goodRoiPrimary( x, y, width, height)
///   Output good ROI: (x,y) coordinates of top left corner and width and height of ROI defines
///   the good region of the primary image that is associated with this depth map.
///   This good ROI should be passed to rendering API with desired effect type
///
/// @param scaleCropRotationDataPrimaryCamera
///   String containing scale, crop and rotation parameters of primary image
///
/// @param scaleCropRotationDataAuxiliaryCamera
///   String containing scale, crop and rotation parameters of auxiliary image
///
/// @param otpCalibraion
///   OTP calibration parameters
///
/// @param focalLengthPrimaryCamera
///   Focal length of Primary camera in millimeters
///
/// @param sensorConfig
///   Pass appropriate configuration from enum SensorConfiguration
///
/// @return
///   DDMWrapperStatus object which contains error code and error message
//------------------------------------------------------------------------------

static inline
DDMWrapperStatus dualCameraGenerateDDM( const uint8_t* primaryY,
                                        const uint8_t* primaryVU,
                                        int primaryWidth,
                                        int primaryHeight,
                                        int primaryStrideY,
                                        int primaryStrideVU,

                                        const uint8_t* auxiliaryY,
                                        const uint8_t* auxiliaryVU,
                                        int auxiliaryWidth,
                                        int auxiliaryHeight,
                                        int auxiliaryStrideY,
                                        int auxiliaryStrideVU,

                                        uint8_t* dst,
                                        int dstStride,

                                        int& goodRoiPrimaryX,
                                        int& goodRoiPrimaryY,
                                        int& goodRoiPrimaryWidth,
                                        int& goodRoiPrimaryHeight,

                                        const std::string& scaleCropRotationDataPrimaryCamera,
                                        const std::string& scaleCropRotationDataAuxiliaryCamera,
                                        const std::string& otpCalibration,
                                        const float focalLengthPrimaryCamera,
                                        SensorConfiguration sensorConfig,
                                        DepthMapMode ddmMode = DepthMapMode::ENHANCED_MODE )
{
   typedef DDMWrapperStatus::ErrorCode ErrorCode;

   // Validate width, height and stride for primary image
   // Image size should be at least 1280x720
   if( std::max( primaryWidth, primaryHeight ) < 1280 ||
       std::min( primaryWidth, primaryHeight ) < 720 )
   {
      return DDMWrapperStatus( ErrorCode::INVALID_ARGUMENT, "Primary Image size less than 1280x720 is not supported" );
   }

   // Width and height should be multiple of 2
   if( ( primaryWidth % 2 ) != 0 || ( primaryHeight % 2 ) != 0 )
   {
      return DDMWrapperStatus( ErrorCode::INVALID_ARGUMENT, "Primary Image width/height should be multiple of 2" );
   }

   // Validate width, height and stride for auxiliary image
   // Image size should be at least 1280x720
   if( std::max( auxiliaryWidth, auxiliaryHeight ) < 1280 ||
       std::min( auxiliaryWidth, auxiliaryHeight ) < 720 )
   {
      return DDMWrapperStatus( ErrorCode::INVALID_ARGUMENT, "Auxiliary Image size less than 1280x720 is not supported" );
   }

   // Width and height should be multiple of 2
   if( ( auxiliaryWidth % 2 ) != 0 || ( auxiliaryHeight % 2 ) != 0 )
   {
      return DDMWrapperStatus( ErrorCode::INVALID_ARGUMENT, "Auxiliary Image width/height should be multiple of 2" );
   }

   // Validate primary image dimensions and buffer
   if( !internal::isInputImageDataValid( primaryY, primaryWidth, primaryHeight, primaryStrideY, 1 ) )
   {
      return DDMWrapperStatus( ErrorCode::INVALID_ARGUMENT, "Invalid PrimaryY image buffer/width/height or stride" );
   }

   if( !internal::isInputImageDataValid( primaryVU, primaryWidth / 2, primaryHeight / 2, primaryStrideVU, 2 ) )
   {
      return DDMWrapperStatus( ErrorCode::INVALID_ARGUMENT, "Invalid PrimaryVU image buffer/width/height or stride" );
   }

   // Validate auxiliary dimensions and buffer
   if( !internal::isInputImageDataValid( auxiliaryY, auxiliaryWidth, auxiliaryHeight, auxiliaryStrideY, 1 ) )
   {
      return DDMWrapperStatus( ErrorCode::INVALID_ARGUMENT, "Invalid auxiliaryY image buffer/width/height or stride" );
   }

   if( !internal::isInputImageDataValid( auxiliaryVU, auxiliaryWidth / 2, auxiliaryHeight / 2, auxiliaryStrideVU, 2 ) )
   {
      return DDMWrapperStatus( ErrorCode::INVALID_ARGUMENT, "Invalid auxiliaryVU image buffer/width/height or stride" );
   }

   //Validate dst dimensions and buffer
   int dstWidth, dstHeight;
   dualcameraddm_qSv3fcF7Tp( primaryWidth, primaryHeight, dstWidth, dstHeight );

   if( !internal::isOutputImageDataValid( dst, dstWidth, dstHeight, dstStride, 1 ) )
   {
      return DDMWrapperStatus( ErrorCode::INVALID_ARGUMENT, "Invalid dst image buffer/width/height or stride" );
   }

   // Validate parameters
   if( scaleCropRotationDataPrimaryCamera.empty() )
   {
      return DDMWrapperStatus( ErrorCode::INVALID_ARGUMENT, "scaleCropRotationDataPrimaryCamera string is empty" );
   }

   if( scaleCropRotationDataAuxiliaryCamera.empty() )
   {
      return DDMWrapperStatus( ErrorCode::INVALID_ARGUMENT, "scaleCropRotationDataAuxiliaryCamera string is empty" );
   }

   if( otpCalibration.empty() )
   {
      return DDMWrapperStatus( ErrorCode::INVALID_ARGUMENT, "otpCalibration string is empty" );
   }

   if( focalLengthPrimaryCamera <= 0 )
   {
      return DDMWrapperStatus( ErrorCode::INVALID_ARGUMENT, "Invalid focalLengthPrimaryCamera" );
   }

   if( (int)sensorConfig < 0 || (int)sensorConfig >= SensorConfiguration::SENSOR_CONFIGURATION_SIZE )
   {
      return DDMWrapperStatus( ErrorCode::INVALID_ARGUMENT, "Invalid Parameters: Sensor configuration not supported" );
   }

   int errCode;
   const char* errMessage = "";

   dualcameraddm_k5DFT9dbFc( primaryY, primaryVU, primaryWidth, primaryHeight, primaryStrideY, primaryStrideVU,
                             auxiliaryY, auxiliaryVU, auxiliaryWidth, auxiliaryHeight, auxiliaryStrideY, auxiliaryStrideVU,
                             dst, dstWidth, dstHeight, dstStride,
                             goodRoiPrimaryX, goodRoiPrimaryY, goodRoiPrimaryWidth, goodRoiPrimaryHeight,
                             scaleCropRotationDataPrimaryCamera.c_str(), scaleCropRotationDataAuxiliaryCamera.c_str(), otpCalibration.c_str(),
                             focalLengthPrimaryCamera, static_cast<int>( sensorConfig ), static_cast<int>( ddmMode ), errCode, errMessage );

   return DDMWrapperStatus( static_cast<ErrorCode>( errCode ), std::string( errMessage ) );
}

//------------------------------------------------------------------------------
/// @brief
///   Outputs the rendering map to be used for rendering 3D Effect
///
/// @param depthMap
///   Input 8-bit single channel depth map
///
/// @param depthMapWidth
///   Width of input depthMap image
///
/// @param depthMapHeight
///   Height of input depthMap image
///
/// @param depthMapStride
///   Stride in bytes of depthMap
///
/// @param renderMap
///   Output render depth map which can be used to render 3D effect
///   Buffer must be single channel
///
/// @param renderMapWidth
///   Desired width of renderMap
///
/// @param renderMapHeight
///   Desired height of renderMap
///
/// @param renderMapStride
///   Stride in bytes of renderMap
///
/// @return
///   DDMWrapperStatus object which contains error code and error message
//------------------------------------------------------------------------------
static inline
DDMWrapperStatus get3DEffectRenderMap( const uint8_t* depthMap,
                                       int depthMapWidth,
                                       int depthMapHeight,
                                       int depthMapStride,
                                       uint8_t* renderMap,
                                       int renderMapWidth,
                                       int renderMapHeight,
                                       int renderMapStride )
{
   // Validate input depth map image dimensions and buffer
   if( !internal::isInputImageDataValid( depthMap, depthMapWidth, depthMapHeight, depthMapStride, 1 ) )
   {
      return DDMWrapperStatus( DDMWrapperStatus::ErrorCode::INVALID_ARGUMENT, "Invalid depth map buffer/width/height or stride" );
   }

   // Validate output image dimensions and buffer
   if( !internal::isOutputImageDataValid( renderMap, renderMapWidth, renderMapHeight, renderMapStride, 1 ) )
   {
      return DDMWrapperStatus( DDMWrapperStatus::ErrorCode::INVALID_ARGUMENT, "Invalid rederMap buffer/width/height or stride" );
   }

   //Call into library
   bool success = dualcameraddm_sKy6qu3UbY( depthMap, depthMapWidth, depthMapHeight, depthMapStride,
                                            renderMap, renderMapWidth, renderMapHeight, renderMapStride );

   if( success )
   {
      return DDMWrapperStatus( DDMWrapperStatus::ErrorCode::OK, "Success" );
   }
   else
   {
      return DDMWrapperStatus( DDMWrapperStatus::ErrorCode::INTERNAL_ERROR, "Internal error in implementation detected" );
   }
}

class CP_DLL_PUBLIC DualCameraDDMEffects
{
public:
   enum EffectType
   {
      INPUT = 0,
      REFOCUS_CIRCLE = 1,
      REFOCUS_HEXAGON = 2,
      REFOCUS_STAR = 3,
      SKETCH = 4,
      HALO = 5,
      MOTION_BLUR = 6,
      FUSION_FOREGROUND = 7,
      ZOOM_BLUR = 8,
      BLACK_WHITE = 9,
      WHITEBOARD = 10,
      BLACKBOARD = 11,
      POSTERIZE = 12,
      NEGATIVE = 13,
      EFFECTYPE_SIZE = 14
   };

   ///-----------------------------------------------------------------------------
   /// @brief
   ///   Constructor: RGBA input format
   ///
   // @param primaryRGBA
   ///   Input 8-bit image from primary camera in RGBA interleaved format
   ///
   /// @param primaryWidth
   ///   Width of input primary image
   ///
   /// @param primaryHeight
   ///   Height of input primary image
   ///
   /// @param primaryStride
   ///   Stride in bytes of primaryRGBA
   ///
   /// @param depthMap
   ///   Input 8-bit single channel depth map
   ///
   /// @param depthMapWidth
   ///   Width of input depthMap image
   ///
   /// @param depthMapHeight
   ///   Height of input depthMap image
   ///
   /// @param depthMapStride
   ///   Stride in bytes of depthMap
   ///
   /// @param goodRoiPrimary( x, y, width, height)
   ///   NOTE: This value should be same as good ROI output from dualCameraGenerateDDM API
   ///   Good ROI: (x,y) coordinates of top left corner and width and height of ROI defines
   ///   the good region of the primary image that is associated with this depth map.
   ///   This good ROI should be passed to rendering API with desired effect type
   ///
   /// @param dstImageWidth
   ///   Width of desired output from renderEffect API
   ///   dst width should be <= goodRoiPrimaryWidth
   ///   This should be even value
   ///
   /// @param dstImageHeight
   ///   Height of desired output from renderEffect API
   ///   dst Height should be <= goodRoiPrimaryHeight
   ///   This should be even value
   ///
   /// @param brightnessIntensity
   ///   Controls amount of brightening. Range: 0 ~ 1.
   ///   Larger value means brighter output.
   ///
   ///-----------------------------------------------------------------------------
   DualCameraDDMEffects( const uint8_t* primaryRGBA,
                         int primaryWidth,
                         int primaryHeight,
                         int primaryStride,

                         const uint8_t* depthMap,
                         int depthMapWidth,
                         int depthMapHeight,
                         int depthMapStride,

                         int goodRoiPrimaryX,
                         int goodRoiPrimaryY,
                         int goodRoiPrimaryWidth,
                         int goodRoiPrimaryHeight,

                         int dstImageWidth,
                         int dstImageHeight,
                         float brightnessIntensity = 0.0f )

                         : _pImpl( NULL ), _dstImageWidth( dstImageWidth ), _dstImageHeight( dstImageHeight )
   {
      typedef DDMWrapperStatus::ErrorCode ErrorCode;

      // Validate width, height and stride for primaryRGBA image
      // Image size should be at least 1280x720
      if( std::max( primaryWidth, primaryHeight ) < 1280 ||
          std::min( primaryWidth, primaryHeight ) < 720 )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Primary Image size less than 1280x720 is not supported" );
         return;
      }

      // Validate primary image dimensions and buffer
      if( !internal::isInputImageDataValid( primaryRGBA, primaryWidth, primaryHeight, primaryStride, 4 ) )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Invalid PrimaryRGBA image buffer/width/height or stride" );
         return;
      }

      // Validate width, height and stride for depthMap image
      // Image size should be at least 1280x720
      if( std::max( depthMapWidth, depthMapHeight ) < 1280 ||
          std::min( depthMapWidth, depthMapHeight ) < 720 )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Depth map size less than 1280x720 is not supported" );
         return;
      }

      // Validate depthMap image dimensions and buffer
      if( !internal::isInputImageDataValid( depthMap, depthMapWidth, depthMapHeight, depthMapStride, 1 ) )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Invalid Depth map buffer/width/height or stride" );
         return;
      }

      // Validate good ROI
      if( goodRoiPrimaryX < 0 || goodRoiPrimaryX > primaryWidth ||
          goodRoiPrimaryY < 0 || goodRoiPrimaryY > primaryHeight ||
          goodRoiPrimaryWidth < 0 || goodRoiPrimaryWidth > primaryWidth ||
          goodRoiPrimaryHeight < 0 || goodRoiPrimaryHeight > primaryHeight ||
          goodRoiPrimaryX + goodRoiPrimaryWidth > primaryWidth ||
          goodRoiPrimaryY + goodRoiPrimaryHeight > primaryHeight )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Invalid input good ROI" );
         return;
      }

      // Validate good ROI
      if( goodRoiPrimaryX % 2 != 0 || goodRoiPrimaryY % 2 != 0 ||
          goodRoiPrimaryWidth % 2 != 0 || goodRoiPrimaryHeight % 2 != 0 )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Invalid input good ROI: ROI should be multiple of 2" );
         return;
      }

      // Validate dst size
      // dstImageWidth should be <= goodRoiPrimaryWidth
      // dstImageHeight should be <= goodRoiPrimaryHeight
      if( dstImageWidth < 0 || dstImageWidth > goodRoiPrimaryWidth ||
          dstImageHeight < 0 || dstImageHeight > goodRoiPrimaryHeight )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Invalid destination image width/height" );
         return;
      }

      if( dstImageWidth % 2 != 0 || dstImageHeight % 2 != 0 )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Destination image width/height should be even" );
         return;
      }

      bool success = _n27g24FFFp( _pImpl, primaryRGBA, primaryWidth, primaryHeight, primaryStride,
                                  depthMap, depthMapWidth, depthMapHeight, depthMapStride,
                                  goodRoiPrimaryX, goodRoiPrimaryY, goodRoiPrimaryWidth, goodRoiPrimaryHeight,
                                  dstImageWidth, dstImageHeight, brightnessIntensity );

      if( success )
      {
         _ctorErrCode = ErrorCode::OK;
         _ctorErrMsg = "DualCameraDDMEffect initialization successful";
      }
      else
      {
         _ctorErrCode = ErrorCode::INIT_FAIL;
         _ctorErrMsg = "DualCameraDDMEffect initialization failed";
      }
   }

   ///-----------------------------------------------------------------------------
   /// @brief
   ///   Constructor: NV21 input format
   ///
   // @param primaryY
   ///   Input single channel luma 8-bit image from primary camera
   ///
   // @param primaryVU
   ///   Input two channel chroma 8-bit image from primary camera
   ///
   /// @param primaryWidth
   ///   Width of input primary image
   ///
   /// @param primaryHeight
   ///   Height of input primary image
   ///
   /// @param primaryYStride
   ///   Stride of primaryY in bytes
   ///
   /// @param primaryVUStride
   ///   Stride of primaryVU in bytes
   ///
   /// @param depthMap
   ///   Input 8-bit single channel depth map
   ///
   /// @param depthMapWidth
   ///   Width of input depthMap image
   ///
   /// @param depthMapHeight
   ///   Height of input depthMap image
   ///
   /// @param depthMapStride
   ///   Stride in bytes of depthMap
   ///
   /// @param goodRoiPrimary( x, y, width, height)
   ///   NOTE: This value should be same as good ROI output from dualCameraGenerateDDM API
   ///   Good ROI: (x,y) coordinates of top left corner and width and height of ROI defines
   ///   the good region of the primary image that is associated with this depth map.
   ///   This good ROI should be passed to rendering API with desired effect type
   ///
   /// @param dstImageWidth
   ///   Width of desired output from renderEffect API
   ///   dst width should be <= goodRoiPrimaryWidth
   ///   This should be even value
   ///
   /// @param dstImageHeight
   ///   Height of desired output from renderEffect API
   ///   dst Height should be <= goodRoiPrimaryHeight
   ///   This should be even value
   ///
   /// @param brightnessIntensity
   ///   Controls amount of brightening. Range: 0 ~ 1.
   ///   Larger value means brighter output.
   ///
   ///-----------------------------------------------------------------------------
   DualCameraDDMEffects( const uint8_t* primaryY,
                         const uint8_t* primaryVU,
                         int primaryWidth,
                         int primaryHeight,
                         int primaryYStride,
                         int primaryVUStride,

                         const uint8_t* depthMap,
                         int depthMapWidth,
                         int depthMapHeight,
                         int depthMapStride,

                         int goodRoiPrimaryX,
                         int goodRoiPrimaryY,
                         int goodRoiPrimaryWidth,
                         int goodRoiPrimaryHeight,

                         int dstImageWidth,
                         int dstImageHeight,
                         float brightnessIntensity = 0.0f )

                         : _pImpl( NULL ), _dstImageWidth( dstImageWidth ), _dstImageHeight( dstImageHeight )
   {
      typedef DDMWrapperStatus::ErrorCode ErrorCode;

      // Validate width, height and stride for primaryRGBA image
      // Image size should be at least 1280x720
      if( std::max( primaryWidth, primaryHeight ) < 1280 ||
         std::min( primaryWidth, primaryHeight ) < 720 )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Primary Image size less than 1280x720 is not supported" );
         return;
      }

      // Width and height should be multiple of 2
      if( ( primaryWidth % 2 ) != 0 || ( primaryHeight % 2 ) != 0 )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Primary Image width/height should be multiple of 2" );
         return;
      }

      // Validate primary image dimensions and buffer
      if( !internal::isInputImageDataValid( primaryY, primaryWidth, primaryHeight, primaryYStride, 1 ) )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Invalid PrimaryY image buffer/width/height or stride" );
         return;
      }

      if( !internal::isInputImageDataValid( primaryVU, primaryWidth / 2, primaryHeight / 2, primaryVUStride, 2 ) )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Invalid PrimaryVU image buffer/width/height or stride" );
         return;
      }

      // Validate width, height and stride for depthMap image
      // Image size should be at least 1280x720
      if( std::max( depthMapWidth, depthMapHeight ) < 1280 ||
         std::min( depthMapWidth, depthMapHeight ) < 720 )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Depth map size less than 1280x720 is not supported" );
         return;
      }

      // Validate depthMap image dimensions and buffer
      if( !internal::isInputImageDataValid( depthMap, depthMapWidth, depthMapHeight, depthMapStride, 1 ) )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Invalid Depth map buffer/width/height or stride" );
         return;
      }

      // Validate good ROI
      if( goodRoiPrimaryX < 0 || goodRoiPrimaryX > primaryWidth ||
         goodRoiPrimaryY < 0 || goodRoiPrimaryY > primaryHeight ||
         goodRoiPrimaryWidth < 0 || goodRoiPrimaryWidth > primaryWidth ||
         goodRoiPrimaryHeight < 0 || goodRoiPrimaryHeight > primaryHeight ||
         goodRoiPrimaryX + goodRoiPrimaryWidth > primaryWidth ||
         goodRoiPrimaryY + goodRoiPrimaryHeight > primaryHeight )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Invalid input good ROI" );
         return;
      }

      // Validate good ROI
      if( goodRoiPrimaryX % 2 != 0 || goodRoiPrimaryY % 2 != 0 ||
          goodRoiPrimaryWidth % 2 != 0 || goodRoiPrimaryHeight % 2 != 0 )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Invalid input good ROI: ROI should be multiple of 2" );
         return;
      }

      // Validate dst size
      // dstImageWidth should be <= goodRoiPrimaryWidth
      // dstImageHeight should be <= goodRoiPrimaryHeight
      if( dstImageWidth < 0 || dstImageWidth > goodRoiPrimaryWidth ||
         dstImageHeight < 0 || dstImageHeight > goodRoiPrimaryHeight )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Invalid destination image width/height" );
         return;
      }

      if( dstImageWidth % 2 != 0 || dstImageHeight % 2 != 0 )
      {
         _ctorErrCode = ErrorCode::INVALID_ARGUMENT;
         _ctorErrMsg = std::string( "Destination image width/height should be even" );
         return;
      }

      bool success = _kW7tcPL7jz( _pImpl, primaryY, primaryVU, primaryWidth, primaryHeight, primaryYStride, primaryVUStride,
                                  depthMap, depthMapWidth, depthMapHeight, depthMapStride,
                                  goodRoiPrimaryX, goodRoiPrimaryY, goodRoiPrimaryWidth, goodRoiPrimaryHeight,
                                  dstImageWidth, dstImageHeight, brightnessIntensity );

      if( success )
      {
         _ctorErrCode = ErrorCode::OK;
         _ctorErrMsg = "DualCameraDDMEffect initialization successful";
      }
      else
      {
         _ctorErrCode = ErrorCode::INIT_FAIL;
         _ctorErrMsg = "DualCameraDDMEffect initialization failed";
      }
   }

   ///-----------------------------------------------------------------------------
   /// @brief
   ///   Call isValid() to check if constructor was initialized succesfully
   ///
   /// @return
   ///   DDMWrapperStatus object which contains error code and error message
   ///-----------------------------------------------------------------------------
   DDMWrapperStatus isValid() const
   {
      return DDMWrapperStatus( _ctorErrCode, _ctorErrMsg );
   }

   //------------------------------------------------------------------------------
   /// @brief
   ///   Generates one word name and short description of post-process effect
   ///
   /// @param dcEffect
   ///   Specify desired effect from enum qrcp::DdmEffects
   ///
   /// @param name
   ///   One word name of the effect
   ///
   /// @param desc
   ///   A short description of the effect
   ///
   //------------------------------------------------------------------------------
   bool getEffectDescription( const EffectType ddmEffect, std::string& name, std::string& desc )
   {
      if( ddmEffect >= EffectType::EFFECTYPE_SIZE )
      {
         return false;
      }

      const char *cName = "";
      const char *cDesc = "";

      _ySf5prRkfD( static_cast<int>( ddmEffect ), cName, cDesc );

      name = std::string( cName );
      desc = std::string( cDesc );

      return true;
   }

   //------------------------------------------------------------------------------
   /// @brief
   ///   Generates selected post-process effect based on depth map generated from dualCameraGenerateDDM API
   ///   Works with RGBA input format
   ///
   /// @param ddmEffect
   ///   Specify desired effect from enum qrcp::DualCameraDDMEffects::EffectType
   ///   Call getEffectDescription API for get brief description of each EffectType
   ///
   ///      INPUT: Generates image which is used as input to all the other effects.
   ///             This image is primaryImage cropped to goodRoi, i.e. (goodRoiPrimaryX, goodRoiPrimaryY, goodRoiPrimaryWidth, goodRoiPrimaryHeight)
   ///             and brightened based on the brightnessIntensity supplied in the constructor.
   ///      REFOCUS_CIRCLE: Generates image focused at specified coordinates and background blurred with circular kernel
   ///      REFOCUS_HEXAGON: Generates image focused at specified coordinates and background blurred with hexagon kernel
   ///      REFOCUS_STAR: Generates image focused at specified coordinates and background blurred with star kernel
   ///      SKETCH: Generates sketch image
   ///      HALO: Generates halo around the object at specified coordinates
   ///      FUSION_FOREGROUND: Generates foreground image( used for background replacement )
   ///      MOTION_BLUR: Generates motion blurred background image
   ///      ZOOM_BLUR: Generates zoom blurred background image
   ///      BLACK_WHITE: Generates black and white background image
   ///      Others effects available: WHITEBOARD, BLACKBOARD, POSTERIZE and NEGATIVE
   ///
   /// @param focusPointX
   ///   X-axis of point of user tap
   ///   Coordinates should be with respect to EffectType::INPUT image, i.e. dstImageWidth passed to the constructor
   ///
   ///   Follow these steps to derive focusPointX using Camera's Auto-focus ROI:
   ///      1.  If "afCenter" = Auto-focus ROI's center point scaled to primaryImage that is passed to the constructor of this class
   ///      2.  Then focusPointX = (afCenter.x - goodRoiPrimaryX) * dstImageWidth / goodRoiPrimaryWidth
   ///
   ///   Note: goodRoiPrimaryWidth and goodRoiPrimaryX are inputs to the constructor of this class
   ///   This is ignored for EffectType::INPUT
   ///
   /// @param focusPointY
   ///   Y-axis of point of user tap
   ///   Coordinates should be with respect to EffectType::INPUT image, i.e. dstImageHeight passed to the constructor
   ///
   ///   Follow these steps to derive focusPointY using Camera's Auto-focus ROI:
   ///      1.  If "afCenter" = Auto-focus ROI's center point scaled to primaryImage that is passed to the constructor of this class
   ///      2.  Then focusPointY = (afCenter.y - goodRoiPrimaryY) * dstImageHeight / goodRoiPrimaryHeight
   ///
   ///   Note: goodRoiPrimaryHeight and goodRoiPrimaryY are inputs to the constructor of this class
   ///   This is ignored for EffectType::INPUT
   ///
   /// @param intensity
   ///   Range: [0-1]
   ///   Specify intensity for following effects:
   ///      REFOCUS : Blurring intensity
   ///      HALO : Halo intensity
   ///      MOTIONBLUR : Motion blur intensity
   ///      POSTERIZE: posterization intensity
   ///      Others: Intensity is ignored
   ///
   /// @param dstRGBA
   ///   Output RGBA interleaved image pointer with element data type of uint8_t
   ///   Note dstRGBA should be pre-allocated and should of size renderEffectImageWidth x renderEffectImageHeight
   ///
   /// @param dstStride
   ///   Stride in bytes of dstRGBA image
   ///
   /// @returns DDMWrapperStatus object which contains error code and error message
   //------------------------------------------------------------------------------
   DDMWrapperStatus renderEffect( EffectType ddmEffect,
                                  int focusPointX,
                                  int focusPointY,
                                  float intensity,
                                  uint8_t* dstRGBA,
                                  int dstStride )
   {
      DDMWrapperStatus ctorStatus = isValid();

      // Check if constructor is initialized successfully
      if( !ctorStatus.ok() )
      {
         return ctorStatus;
      }

      if( ddmEffect < 0 || ddmEffect >= EffectType::EFFECTYPE_SIZE )
      {
         return DDMWrapperStatus( DDMWrapperStatus::ErrorCode::INVALID_ARGUMENT, "Input effect type doesn't match supported type" );
      }

      // dstRGBA size must be same as renderEffectImageWidth x renderEffectImageHeight passed in constructor
      if( !internal::isOutputImageDataValid( dstRGBA, _dstImageWidth, _dstImageHeight, dstStride, 4 ) )
      {
         return DDMWrapperStatus( DDMWrapperStatus::ErrorCode::INVALID_ARGUMENT, "Invalid dstRGBA image buffer/width/height or stride" );
      }

      if( focusPointX < 0 || focusPointX >= _dstImageWidth || focusPointY < 0 || focusPointY >= _dstImageHeight )
      {
         return DDMWrapperStatus( DDMWrapperStatus::ErrorCode::INVALID_ARGUMENT, "Invalid focusPointX or focusPointY" );
      }

      int errCode;
      const char* errMessage = "";

      _cZ38NjVp6X( _pImpl, static_cast<int>( ddmEffect ), focusPointX, focusPointY, intensity, dstRGBA, _dstImageWidth, _dstImageHeight, dstStride, errCode, errMessage );

      return DDMWrapperStatus( static_cast<DDMWrapperStatus::ErrorCode>( errCode ), std::string( errMessage ) );
   }

   //------------------------------------------------------------------------------
   /// @brief
   ///   Generates selected post-process effect based on depth map generated from dualCameraGenerateDDM API
   ///   Works with NV21 input format
   ///
   /// @Note
   ///   To get alpha channel for FUSION_FOREGROUND, call renderEffect API with RGBA output
   ///
   /// @param ddmEffect
   ///   Specify desired effect from enum qrcp::DualCameraDDMEffects::EffectType
   ///   Call getEffectDescription API for get brief description of each EffectType
   ///
   ///      INPUT: Generates image which is used as input to all the other effects.
   ///             This image is primaryImage cropped to goodRoi, i.e. (goodRoiPrimaryX, goodRoiPrimaryY, goodRoiPrimaryWidth, goodRoiPrimaryHeight)
   ///             and brightened based on the brightnessIntensity supplied in the constructor.
   ///      REFOCUS_CIRCLE: Generates image focused at specified coordinates and background blurred with circular kernel
   ///      REFOCUS_HEXAGON: Generates image focused at specified coordinates and background blurred with hexagon kernel
   ///      REFOCUS_STAR: Generates image focused at specified coordinates and background blurred with star kernel
   ///      SKETCH: Generates sketch image
   ///      HALO: Generates halo around the object at specified coordinates
   ///      FUSION_FOREGROUND: Generates foreground image( used for background replacement )
   ///      MOTION_BLUR: Generates motion blurred background image
   ///      ZOOM_BLUR: Generates zoom blurred background image
   ///      BLACK_WHITE: Generates black and white background image
   ///      Others effects available: WHITEBOARD, BLACKBOARD, POSTERIZE and NEGATIVE
   ///
   /// @param focusPointX
   ///   X-axis of point of user tap
   ///   Coordinates should be with respect to EffectType::INPUT image, i.e. dstImageWidth passed to the constructor
   ///
   ///   Follow these steps to derive focusPointX using Camera's Auto-focus ROI:
   ///      1.  If "afCenter" = Auto-focus ROI's center point scaled to primaryImage that is passed to the constructor of this class
   ///      2.  Then focusPointX = (afCenter.x - goodRoiPrimaryX) * dstImageWidth / goodRoiPrimaryWidth
   ///
   ///   Note: goodRoiPrimaryWidth and goodRoiPrimaryX are inputs to the constructor of this class
   ///   This is ignored for EffectType::INPUT
   ///
   /// @param focusPointY
   ///   Y-axis of point of user tap
   ///   Coordinates should be with respect to EffectType::INPUT image, i.e. dstImageHeight passed to the constructor
   ///
   ///   Follow these steps to derive focusPointY using Camera's Auto-focus ROI:
   ///      1.  If "afCenter" = Auto-focus ROI's center point scaled to primaryImage that is passed to the constructor of this class
   ///      2.  Then focusPointY = (afCenter.y - goodRoiPrimaryY) * dstImageHeight / goodRoiPrimaryHeight
   ///
   ///   Note: goodRoiPrimaryHeight and goodRoiPrimaryY are inputs to the constructor of this class
   ///   This is ignored for EffectType::INPUT
   ///
   /// @param intensity
   ///   Range: [0-1]
   ///   Specify intensity for following effects:
   ///      REFOCUS : Blurring intensity
   ///      HALO : Halo intensity
   ///      MOTIONBLUR : Motion blur intensity
   ///      POSTERIZE: posterization intensity
   ///      Others: Intensity is ignored
   ///
   /// @param dstY
   ///   Output luma image pointer with element data type of uint8_t
   ///   Note dstY should be 1 channel pre-allocated buffer and should of size dstImageWidth x dstImageHeight
   ///
   /// @param dstVU
   ///   Output chroma image pointer with element data type of uint8_t
   ///   Note dstVU should be 2 channel pre-allocated buffer and should of size dstImageWidth/2 x dstImageHeight/2
   ///
   /// @param dstYStride
   ///   Stride of dstY in bytes
   ///
   /// @param dstVUStride
   ///   Stride of dstVU in bytes
   ///
   /// @returns DDMWrapperStatus object which contains error code and error message
   //------------------------------------------------------------------------------
   DDMWrapperStatus renderEffect( EffectType ddmEffect,
                                  int focusPointX,
                                  int focusPointY,
                                  float intensity,
                                  uint8_t* dstY,
                                  uint8_t* dstVU,
                                  int dstYStride,
                                  int dstVUStride )
   {
      DDMWrapperStatus ctorStatus = isValid();

      // Check if constructor is initialized successfully
      if( !ctorStatus.ok() )
      {
         return ctorStatus;
      }

      if( ddmEffect < 0 || ddmEffect >= EffectType::EFFECTYPE_SIZE )
      {
         return DDMWrapperStatus( DDMWrapperStatus::ErrorCode::INVALID_ARGUMENT, "Input effect type doesn't match supported type" );
      }

      // dstRGBA size must be same as renderEffectImageWidth x renderEffectImageHeight passed in constructor
      if( !internal::isOutputImageDataValid( dstY, _dstImageWidth, _dstImageHeight, dstYStride, 1 ) )
      {
         return DDMWrapperStatus( DDMWrapperStatus::ErrorCode::INVALID_ARGUMENT, "Invalid dstY image buffer/width/height or stride" );
      }

      // dstRGBA size must be same as renderEffectImageWidth x renderEffectImageHeight passed in constructor
      if( !internal::isOutputImageDataValid( dstVU, _dstImageWidth / 2, _dstImageHeight / 2, dstVUStride, 2 ) )
      {
         return DDMWrapperStatus( DDMWrapperStatus::ErrorCode::INVALID_ARGUMENT, "Invalid dstVU image buffer/width/height or stride" );
      }

      if( focusPointX < 0 || focusPointX >= _dstImageWidth || focusPointY < 0 || focusPointY >= _dstImageHeight )
      {
         return DDMWrapperStatus( DDMWrapperStatus::ErrorCode::INVALID_ARGUMENT, "Invalid focusPointX or focusPointY" );
      }

      int errCode;
      const char* errMessage = "";

      _kpvUScvK8u( _pImpl, static_cast<int>( ddmEffect ), focusPointX, focusPointY, intensity, dstY, dstVU, _dstImageWidth, _dstImageHeight, dstYStride, dstVUStride, errCode, errMessage );

      return DDMWrapperStatus( static_cast<DDMWrapperStatus::ErrorCode>( errCode ), std::string( errMessage ) );
   }

   // Destructor
   ~DualCameraDDMEffects()
   {
      _nmMkwDL9CQ( _pImpl );
   }

private:
   DualCameraDDMEffects(); // no default constructor
   DualCameraDDMEffects( const DualCameraDDMEffects& ); // no copy constructor
   DualCameraDDMEffects& operator=( const DualCameraDDMEffects& ); // no copy assignment operator

   static
   bool _n27g24FFFp( DualCameraEffects*&, const uint8_t*, int, int, int,
                     const uint8_t*, int, int, int,
                     int, int, int, int, int, int, float );

   static
   bool _kW7tcPL7jz( DualCameraEffects*&, const uint8_t*, const uint8_t*, int, int, int,
                     int, const uint8_t*, int, int,
                     int, int, int, int, int, int, int, float );

   static
   void _cZ38NjVp6X( DualCameraEffects*, int, int, int, float, uint8_t*, int, int, int, int&, const char*& );

   static
   void _kpvUScvK8u( DualCameraEffects*, int, int, int, float, uint8_t*, uint8_t*, int, int, int, int, int&, const char*& );

   static
   void _nmMkwDL9CQ( DualCameraEffects*& );

   static
   void _ySf5prRkfD( const int, const char*&, const char*& );

   DDMWrapperStatus::ErrorCode _ctorErrCode;
   std::string _ctorErrMsg;

   DualCameraEffects* _pImpl;
   const int _dstImageWidth, _dstImageHeight;
};

//==============================================================================
// Following API(s) are now deprecated
// They will be removed in future release
//==============================================================================

//------------------------------------------------------------------------------
/// @brief
///   Generate depth map from primary and auxiliary NV21 images
///
/// @param primaryY
///   Single channel 8 bit luma channel of primary image
///
/// @param primaryVU
///   Two channel 8 bit chroma channels of primary image
///
/// @param primaryStrideY
///   Stride of luma primary image
///
/// @param primaryStrideVU
///   Stride of chroma primary image
///
/// @param auxiliaryY
///   Single channel 8 bit luma channel of auxiliary image
///
/// @param auxiliaryVU
///   Two channel 8 bit chroma channels of auxiliary image
///
/// @param auxiliaryStrideY
///   Stride of luma auxiliary image
///
/// @param auxiliaryStrideVU
///   Stride of chroma auxiliary image
///
/// @param dst
///   Output depth map
///   Pre-allocated dst buffer should be single channel 8 bit and of correct depth map size
///   Get depth map size by calling getDepthMapSize() API before calling this function
///
/// @param dstStride
///   Stride of dst buffer
///
/// @param goodRoiPrimary( x, y, width, height)
///   Output good ROI: (x,y) coordinates of top left corner and width and height of ROI defines
///   the good region of the primary image that is associated with this depth map.
///   This good ROI should be passed to rendering API with desired effect type
///
/// @param scaleCropRotationDataPrimaryCamera
///   String containing scale, crop and rotation parameters of primary image
///
/// @param scaleCropRotationDataAuxiliaryCamera
///   String containing scale, crop and rotation parameters of auxiliary image
///
/// @param otpCalibraion
///   OTP calibration parameters
///
/// @param focalLengthPrimaryCamera
///   Focal length of Primary camera in millimeters
///
/// @param isAuxiliaryMonoSensor
///   Set to true if auxiliary image is taken from a monochrome sensor
///
/// @return
///   DDMWrapperStatus object which contains error code and error message
//------------------------------------------------------------------------------
static inline
DDMWrapperStatus dualCameraGenerateDDM( const uint8_t* primaryY,
                                        const uint8_t* primaryVU,
                                        int primaryWidth,
                                        int primaryHeight,
                                        int primaryStrideY,
                                        int primaryStrideVU,

                                        const uint8_t* auxiliaryY,
                                        const uint8_t* auxiliaryVU,
                                        int auxiliaryWidth,
                                        int auxiliaryHeight,
                                        int auxiliaryStrideY,
                                        int auxiliaryStrideVU,

                                        uint8_t* dst,
                                        int dstStride,

                                        int& goodRoiPrimaryX,
                                        int& goodRoiPrimaryY,
                                        int& goodRoiPrimaryWidth,
                                        int& goodRoiPrimaryHeight,

                                        const std::string& scaleCropRotationDataPrimaryCamera,
                                        const std::string& scaleCropRotationDataAuxiliaryCamera,
                                        const std::string& otpCalibration,
                                        const float focalLengthPrimaryCamera,
                                        bool isAuxiliaryMonoSensor,
                                        DepthMapMode ddmMode = DepthMapMode::ENHANCED_MODE )
{
   SensorConfiguration sensorConfig = isAuxiliaryMonoSensor ? SensorConfiguration::SYMMETRIC_BAYER_MONO : SENSOR_CONFIGURATION_SIZE;

   return dualCameraGenerateDDM( primaryY, primaryVU, primaryWidth, primaryHeight, primaryStrideY, primaryStrideVU,
                                 auxiliaryY, auxiliaryVU, auxiliaryWidth, auxiliaryHeight, auxiliaryStrideY, auxiliaryStrideVU,
                                 dst, dstStride,
                                 goodRoiPrimaryX, goodRoiPrimaryY, goodRoiPrimaryWidth, goodRoiPrimaryHeight,
                                 scaleCropRotationDataPrimaryCamera, scaleCropRotationDataAuxiliaryCamera, otpCalibration,
                                 focalLengthPrimaryCamera, sensorConfig, ddmMode );
}

} // namespace qrcp

#endif
