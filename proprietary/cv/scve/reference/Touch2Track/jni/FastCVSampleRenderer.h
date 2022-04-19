#ifndef FAST_CV_SAMPLE_RENDERER_H
#define FAST_CV_SAMPLE_RENDERER_H

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
//#include "rect.h"
#include <fastcv_.h>

//==============================================================================
// Declarations
//==============================================================================
extern "C"
{
//---------------------------------------------------------------------------
/// @brief
///   Sets up the viewport
/// @param width
///    width of viewport
/// @param height
///    height of viewport
//---------------------------------------------------------------------------
JNIEXPORT void JNICALL
Java_com_qualcomm_qti_touch2track_FastCVSampleRenderer_surfaceChanged
(
   JNIEnv *env,
   jobject obj,
   jint    width,
   jint    height
   );

//---------------------------------------------------------------------------
/// @brief
///   Cleans up any memory and state associated with the renderer.
//---------------------------------------------------------------------------
JNIEXPORT void JNICALL
Java_com_qualcomm_qti_touch2track_FastCVSampleRenderer_cleanup
(
   JNIEnv *env,
   jobject obj
   );

//---------------------------------------------------------------------------
/// @brief
///   Update the display
/// @param img
///    pointer to buffer that is to be rendered
/// @param w
///    width of buffer
/// @param h
///    height of buffer
//---------------------------------------------------------------------------
JNIEXPORT void JNICALL
Java_com_qualcomm_qti_touch2track_FastCVSampleRenderer_render
(
   JNIEnv *env,
   jobject obj,
   jbyteArray img,
   jint w,
   jint h
   );

//------------------------------------------------------------------------------
/// @brief Gets the rendering frame rate
///
/// @return Rendering FPS (frames per second)
//------------------------------------------------------------------------------
JNIEXPORT jfloat JNICALL
Java_com_qualcomm_qti_touch2track_FastCVSampleRenderer_getGPUFPS
(
   JNIEnv *env,
   jobject  obj
   );

//---------------------------------------------------------------------------
/// @brief
///   Retrieves a pointer to the renderer's rendering buffer, will create
///   a new buffer if not yet allocated or size changes. Locks buffer so
///   don't call lock before this.
///
/// @param w Width of image to render.
/// @param h Height of image to render.
///
/// @return Pointer to buffer.
//---------------------------------------------------------------------------
uint8_t* getRenderBuffer(uint32_t w, uint32_t h);

//---------------------------------------------------------------------------
/// @brief
///   Returns the size of the render buffer for the buffer's format.
///
/// @return Size in bytes of the render buffer.
//---------------------------------------------------------------------------
uint32_t getRenderBufferSize();

//---------------------------------------------------------------------------
/// @brief
///   Unlocks render buffer for use.
///
//---------------------------------------------------------------------------
void unlockRenderBuffer();

//---------------------------------------------------------------------------
/// @brief
///   Locks render buffer to prevent processing collisions and data
///   corruption.
//---------------------------------------------------------------------------
void lockRenderBuffer();

//---------------------------------------------------------------------------
/// @brief
///   Draws specified corners on the render buffer.
///
/// @param corners pointer to corner data stored as unsigned int's
///                x0,y0,x1,y1....
/// @param numCorners Number of corners found in corners.
///
//---------------------------------------------------------------------------
void drawCorners(uint32_t *corners, uint32_t numCorners, char color = 'g', int lineWidth = 7);

//---------------------------------------------------------------------------
/// @brief
///   Draws tracking bounding boxes on the render buffer.
///
/// @param tracking bounding boxes stored as unsigned int's
///                x,y,w,h....
///
//---------------------------------------------------------------------------
void drawBoundingBox(int x, int y, int w, int h, char color = 'g', int lineWidth = 5);


//---------------------------------------------------------------------------
/// @brief
///   zoom in (or out) an src bbox to dest Bbox on the renderer buffer.
///
/// @param source bounding box (x,y,w,h)
/// @param destination bounding box (x,y,w,h)
/// @param step in x direction for the srcBbox
/// @param step in y direction for the srcBbox
///
//---------------------------------------------------------------------------
void imagePatchZoom(fcvRectangleInt &srcBbox, fcvRectangleInt &destBbox, float xstep, float ystep);


};

#endif // FAST_CV_SAMPLE_H
