#ifndef FAST_CV_SAMPLE_H
#define FAST_CV_SAMPLE_H

/*
 * Copyright (c) 2010-2011, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *
 * @file Definition of FastCV Sample Java Native Interface (JNI).
 *
 */

//==============================================================================
// Include Files
//==============================================================================
#include <jni.h>
#include "scveTypes.hpp"
#include "scveContext.hpp"
#include "scveObjectTracker.hpp"
#include "scveUtils.hpp"
#include "scveALContext.hpp"

//==============================================================================
// Declarations
//==============================================================================
extern "C"
{

//---------------------------------------------------------------------------
/// @brief
///    Initializes the tracker
//---------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_init
(
   JNIEnv   *env,
   jobject  obj,
   jint     w,
   jint     h,
   jboolean yuvEnabled,
   jint     trackerMode,
   jboolean useAsync
);

//---------------------------------------------------------------------------
/// @brief
///   Cleans up the tracker
//---------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_cleanup
(
   JNIEnv *env,
   jobject obj
);

//------------------------------------------------------------------------------
/// @brief Calls FastCV API to find Corners
///        Additionally this function overlays pixels on the camera preview
///        image
/// @param img Pointer to camera image
/// @param w Width of image
/// @param h Height of height
//------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_update
(
   JNIEnv*     env,
   jobject     obj,
   jbyteArray  img,
   jintArray   jTrackingResultArray
);

//------------------------------------------------------------------------------
/// @brief Gets the camera frame rate
///
/// @return Camera FPS (frames per second)
//------------------------------------------------------------------------------
JNIEXPORT jfloat JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_getCameraFPS
(
   JNIEnv *env,
   jobject  obj
   );

//------------------------------------------------------------------------------
/// @brief Increments the camera frame tick count
///
//------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_cameraFrameTick
(
   JNIEnv *env,
   jobject  obj
   );

//------------------------------------------------------------------------------
/// @brief Retrieves FastCV processing timing from native layer.
///
/// @return FastCV processing timing, filtered, in ms.
//------------------------------------------------------------------------------
JNIEXPORT jfloat JNICALL
Java_com_qualcomm_qti_touch2track_FastCVSample_getFastCVProcessTime
(
   JNIEnv *env,
   jobject  obj
);

JNIEXPORT int
JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_setROIext
(
   JNIEnv*     env,
   jobject     obj,
   jbyteArray  img,
   int         x,
   int         y,
   int         width,
   int         height,
   jintArray   jObjectHandleArray
);

JNIEXPORT int
JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_setROIbyPointext
(
   JNIEnv*     env,
   jobject     obj,
   jbyteArray  img,
   int         x,
   int         y,
   int        newRoiFlag,
   jintArray   jObjectHandlePointArray
);

JNIEXPORT void
JNICALL Java_com_qualcomm_qti_touch2track_FastCVSample_unregisterROI
(
   JNIEnv*     env,
   jobject     obj,
   int         index
);


};


#endif // FAST_CV_SAMPLE_H
