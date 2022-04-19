#ifndef VIDEOBOKEH_WRAPPER_H
#define VIDEOBOKEH_WRAPPER_H

/**=============================================================================
*  Copyright (c) 2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================**/


//==============================================================================
// Included modules
//==============================================================================
#include <stdint.h>
#include <limits>

//==============================================================================
// MACROS
//==============================================================================
#ifdef _MSC_VER
#define CP_DLL_PUBLIC __declspec(dllexport)
#else
#define CP_DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif

//==============================================================================
// DECLARATIONS
//==============================================================================
class VideoBokeh;


namespace qrcp
{

   namespace internal
   {

      //==============================================================================
      // UTILITY FUNCTIONS
      //==============================================================================
      static inline bool isImageDimensionsValid( int width, int height, int stride, int channels )
      {
         if ( width <= 0 || width > static_cast<int>( std::numeric_limits<int>::max() ) )
         {
            return false;
         }

         if ( height <= 0 || height > static_cast<int>( std::numeric_limits<int>::max() ) )
         {
            return false;
         }

         if ( stride <= 0 || stride < width * channels || stride > static_cast<int>( std::numeric_limits<int>::max() ) )
         {
            return false;
         }

         return true;
      }

      static inline bool isImageDataValid( const uint8_t* imgPtr, int width, int height,
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

      static inline bool isImageDataValid( uint8_t* imgPtr, int width, int height,
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
         imgPtr[0] = 0;
         imgPtr[stride * height - 1] = 0;

         return true;
      }
   } // internal namespace


   //------------------------------------------------------------------------------
   /// @brief
   ///   Status descriptor for VideoBokeh
   //------------------------------------------------------------------------------
   class CP_DLL_PUBLIC VideoBokehWrapperStatus
   {
   public:
      enum ErrorCode
      {
         OK = 0,                        // Everything is Ok. No error
         INVALID_ARGUMENT = -1,         // Function received an invalid argument
         INIT_FAIL = -2,                // Initialization failed
         INTERNAL_ERROR = -3,           // Internal error occured in implementation
         TOO_NEAR = -4,                 // Camera is too close to object for DDM calculation
         TOO_FAR = -5,                  // Camera is too far from object for DDM calculation
         FASTRPC_CONNRESET = -6,        // FastRPC connection reset error
      };

      enum ProcessorType
      {
         HVX = 0,             // Ran on HVX
         CPU = 1              // Ran on CPU
      };

      ///-----------------------------------------------------------------------------
      /// @brief
      ///  Constructor
      ///-----------------------------------------------------------------------------
      VideoBokehWrapperStatus() : _code( ErrorCode::INIT_FAIL ), _errMsg( "Default Initialization" ), _procType( ProcessorType::CPU ) {}

      ///-----------------------------------------------------------------------------
      /// @brief
      ///  Constructor
      ///-----------------------------------------------------------------------------
      VideoBokehWrapperStatus( const ErrorCode& code, const std::string& errMsg, const ProcessorType& procType = ProcessorType::CPU ) : _code( code ), _errMsg( errMsg ), _procType( procType ) {}

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

      ///-----------------------------------------------------------------------------
      /// @brief
      ///   Get which processor the process was run on
      ///-----------------------------------------------------------------------------
      ProcessorType getProcessorType() const
      {
         return _procType;
      }

   private:
      ErrorCode _code;
      std::string _errMsg;
      ProcessorType _procType;
   };

   //==============================================================================
   // API CLASS
   //==============================================================================
   class CP_DLL_PUBLIC VideoBokehWrapper
   {
   public:

      //------------------------------------------------------------------------------
      /// @brief
      ///   Constructor
      ///
      /// @param primaryWidth
      ///   Width of primary image to be processed
      ///
      /// @param primaryHeight
      ///   Height of primary image to be processed
      ///
      /// @param auxiliaryWidth
      ///   Height of auxiliary image to be processed
      ///
      /// @param auxiliaryHeight
      ///   Height of auxiliary image to be processed
      ///
      /// @param scaleCropRotationDataPrimaryCamera
      ///   String containing scale-crop data of primary camera
      ///
      /// @param scaleCropRotationDataPrimaryCameraSize
      ///   Size of string scaleCropRotationDataPrimaryCamera
      ///
      /// @param scaleCropRotationDataAuxiliaryCamera
      ///   String containing scale-crop data of auxiliary camera
      ///
      /// @param scaleCropRotationDataAuxiliaryCameraSize
      ///   Size of string scaleCropRotationDataAuxiliaryCamera
      ///
      /// @param otpCalibration
      ///   String containing OTP calibration data
      ///
      /// @param otpCalibrationSize
      ///   Size of string otpCalibration
      ///
      /// @param primaryFocalLengthMM
      ///   Focal length of primary camera in milli meters
      //------------------------------------------------------------------------------
      VideoBokehWrapper( int primaryWidth,
                         int primaryHeight,
                         int auxiliaryWidth,
                         int auxiliaryHeight,
                         const char scaleCropRotationDataPrimaryCamera[],
                         int scaleCropRotationDataPrimaryCameraSize,
                         const char scaleCropRotationDataAuxiliaryCamera[],
                         int scaleCropRotationDataAuxiliaryCameraSize,
                         const char otpCalibration[],
                         int otpCalibrationSize,
                         float primaryFocalLengthMM,
                         bool enableHVX = false ) :
                         _pImpl(NULL), _primaryWidth(primaryWidth), _primaryHeight(primaryHeight), _auxiliaryWidth(auxiliaryWidth), _auxiliaryHeight(auxiliaryHeight)
      {
         typedef VideoBokehWrapperStatus::ErrorCode ErrorCode;

         // Image size should be at least VGA
         if ( std::max( _primaryWidth, _primaryHeight ) < 640 ||
              std::min( _primaryWidth, _primaryHeight ) < 480 ||
              std::max( _auxiliaryWidth, _auxiliaryHeight ) < 640 ||
              std::min( _auxiliaryWidth, _auxiliaryHeight ) < 480 )
         {
            _ctorErrCode = ErrorCode::INIT_FAIL;
            _ctorErrMsg = "Input width/heigh too small";
            return;
         }

         // Aspect ratio of primary and auxilary image should be same
         if ( ( (float)_primaryWidth / _primaryHeight ) != ( (float)_auxiliaryWidth / _auxiliaryHeight ) )
         {
            _ctorErrCode = ErrorCode::INIT_FAIL;
            _ctorErrMsg = "Aspect ratio of primary and auxiliary is not same";
            return;
         }

         int errorCode;
         _z6eBNA9qVk( _pImpl, _primaryWidth, _primaryHeight, _auxiliaryWidth, _auxiliaryHeight,
                      scaleCropRotationDataPrimaryCamera, scaleCropRotationDataPrimaryCameraSize,
                      scaleCropRotationDataAuxiliaryCamera, scaleCropRotationDataAuxiliaryCameraSize,
                      otpCalibration, otpCalibrationSize, primaryFocalLengthMM, enableHVX,
                      _dstWidth, _dstHeight, errorCode, _ctorErrMsg );

         _ctorErrCode = static_cast<VideoBokehWrapperStatus::ErrorCode>( errorCode );
      }

      //------------------------------------------------------------------------------
      /// @brief
      ///   Returns status of constructor initialization
      //------------------------------------------------------------------------------
      VideoBokehWrapperStatus getConstructorStatus() const
      {
         return VideoBokehWrapperStatus( _ctorErrCode, _ctorErrMsg );
      }

      //------------------------------------------------------------------------------
      /// @brief
      ///   Destructor
      //------------------------------------------------------------------------------
      ~VideoBokehWrapper()
      {
         _buFnq8wz4U( _pImpl );
      }

      //------------------------------------------------------------------------------
      /// @brief
      ///   Get rendering parameters
      ///
      /// @details
      ///   This function returns the parameters needed for rendering
      ///
      /// @param dstWidth
      ///   Width of destination depth map buffer
      ///
      /// @param dstHeight
      ///   Height of destination depth map buffer
      ///
      /// @param goodROISrc(x, y, Width, Height)
      ///   (x,y) coordiates of top left corner and width and height of ROI that
      ///   defines the good region of the primary image. ROI is provided to mark the
      ///   borders of the primary image that should be used for rendering.
      ///
      /// @return
      ///   True if successful; false otherwise
      //------------------------------------------------------------------------------
      bool getRenderParams( int& dstWidth, int& dstHeight,
                            int& goodROISrcx, int& goodROISrcy,
                            int& goodROISrcWidth, int& goodROISrcHeight )
      {
         if( _pImpl == NULL )
         {
            return false;
         }

         dstWidth = _dstWidth;
         dstHeight = _dstHeight;

         _eWq2M7bPJZ( _pImpl, goodROISrcx, goodROISrcy, goodROISrcWidth, goodROISrcHeight );

         return true;
      }

      //------------------------------------------------------------------------------
      /// @brief
      ///   Process one input NV21 frame and produce one output NV21 frame
      ///
      /// @details
      ///   This function removes haze in the input frame to produce the output frame
      ///
      /// @param srcYPtr
      ///   Luma channel of input
      ///
      /// @param srcVUPtr
      ///   Chroma channel of input
      ///
      /// @param srcYStride
      ///   Stride of luma input
      ///
      /// @param srcVUStride
      ///   Stride of chroma input
      ///
      /// @param dstYPtr
      ///   Luma channel of output
      ///
      /// @param dstVUPtr
      ///   Chroma channel of output
      ///
      /// @param dstYStride
      ///   Stride of luma output
      ///
      /// @param dstVUStride
      ///   Stride of chroma output
      ///
      /// @param focusPointX
      ///   x coordinate of point of focus
      ///
      /// @param focusPointY
      ///   y coordinate of point of focus
      ///
      /// @param bokehIntensity
      ///   Strength of bokeh. Range between 0 and 1. Higher value means more bokeh
      ///
      /// @param focalLengthRatioPrimaryCamera
      ///   Focal length ratio of primary camera
      ///
      /// @param focalLengthRatioAuxiliaryCamera
      ///   Focal length ratio of auxiliary camera
      ///
      /// @return
      ///   True if successful; false otherwise
      //------------------------------------------------------------------------------
      VideoBokehWrapperStatus processFrame( const uint8_t* primaryYPtr, int primaryYStride,
                                            const uint8_t* auxiliaryYPtr, int auxiliaryYStride,
                                            uint8_t* dstYPtr, int dstYStride,
                                            int focusPointX, int focusPointY, float bokehIntensity,
                                            float focalLengthRatioPrimaryCamera, float focalLengthRatioAuxiliaryCamera )
      {
         // Check if constructor is initialized successfully
         VideoBokehWrapperStatus ctorStatus = getConstructorStatus();
         if ( !ctorStatus.ok() )
         {
            return ctorStatus;
         }

         if( _pImpl == NULL )
         {
            return VideoBokehWrapperStatus( VideoBokehWrapperStatus::INIT_FAIL, "Initialization failed" );
         }

         // Validate primaryYPtr dimensions and buffer
         if ( !internal::isImageDataValid( primaryYPtr, _primaryWidth, _primaryHeight, primaryYStride, 1 ) )
         {
            return VideoBokehWrapperStatus( VideoBokehWrapperStatus::INVALID_ARGUMENT, "Primary width/height/stride invalid" );
         }

         // Validate auxiliaryYPtr dimensions and buffer
         if ( !internal::isImageDataValid( auxiliaryYPtr, _auxiliaryWidth, _auxiliaryHeight, auxiliaryYStride, 1 ) )
         {
            return VideoBokehWrapperStatus( VideoBokehWrapperStatus::INVALID_ARGUMENT, "Auxiliary width/height/stride invalid" );
         }

         //Validate dstYPtr dimensions and buffer
         if ( !internal::isImageDataValid( dstYPtr, _dstWidth, _dstHeight, dstYStride, 1 ) )
         {
            return VideoBokehWrapperStatus( VideoBokehWrapperStatus::INVALID_ARGUMENT, "Destination width/height/stride invalid" );
         }

         // Validate focal length ratio
         if( focalLengthRatioPrimaryCamera < 1.0f || focalLengthRatioAuxiliaryCamera < 1.0f )
         {
            return VideoBokehWrapperStatus( VideoBokehWrapperStatus::INVALID_ARGUMENT, "Focal length ratio invalid" );
         }


         // Validate focus point
         if ( focusPointX < 0 || focusPointX >= _primaryWidth || focusPointY < 0 || focusPointY >= _primaryHeight )
         {
            return VideoBokehWrapperStatus( VideoBokehWrapperStatus::INVALID_ARGUMENT, "Focus point not valid" );
         }

         // Validate intensity
         if( bokehIntensity < 0 || bokehIntensity > 1.0f )
         {
            return VideoBokehWrapperStatus( VideoBokehWrapperStatus::INVALID_ARGUMENT, "Bokeh intensity not valid" );
         }

         const char *errorString = "";
         int errorCode;
         int procType;
         _k2nhhs59TG( _pImpl,
                      primaryYPtr, _primaryWidth, _primaryHeight, primaryYStride,
                      auxiliaryYPtr, _auxiliaryWidth, _auxiliaryHeight, auxiliaryYStride,
                      dstYPtr, _dstWidth, _dstHeight, dstYStride,
                      focusPointX, focusPointY, bokehIntensity,
                      focalLengthRatioPrimaryCamera, focalLengthRatioAuxiliaryCamera,
                      errorCode, procType, errorString );
         return VideoBokehWrapperStatus( static_cast<VideoBokehWrapperStatus::ErrorCode>( errorCode ), std::string( errorString ), static_cast<VideoBokehWrapperStatus::ProcessorType>( procType ) );
      }


   private:
      VideoBokehWrapper(); // no default constructor
      VideoBokehWrapper( const VideoBokehWrapper& ); // no copy constructor
      VideoBokehWrapper& operator=( const VideoBokehWrapper& ); // no copy assignment operator

      static void _z6eBNA9qVk( VideoBokeh*&, int, int, int, int, const char[], int, const char[],
                               int, const char[], int, float, bool,
                               int&, int&, int&, const char *& );

      static void _eWq2M7bPJZ( VideoBokeh*, int&, int&, int&, int& );

      static void _buFnq8wz4U( VideoBokeh* );


      static void _k2nhhs59TG( VideoBokeh*, const uint8_t*, int, int, int, const uint8_t*,
                               int, int, int, uint8_t* dstYPtr, int, int, int, int, int,
                               float, float, float, int&, int&, const char *& );

      VideoBokeh* _pImpl;
      const int _primaryWidth, _primaryHeight;
      const int _auxiliaryWidth, _auxiliaryHeight;
      int _dstWidth, _dstHeight;

      VideoBokehWrapperStatus::ErrorCode _ctorErrCode;
      const char *_ctorErrMsg;
   };

} // namespace qrcp

#endif
