/*
 * Copyright (c) 2010-2011, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *
 * @file Implementation of FastCV Sample native interface.
 *
 */

//==============================================================================
// Include Files
//==============================================================================
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdlib.h>
#include <android/log.h>
#include <time.h>
#include <pthread.h>
#include <fastcv.h>
#include <fastcv_.h>
#include "rpcmem.h"
#include "FastCVSampleRenderer.h"
#include "CameraRendererRGB565GL2.h"
#include "FPSCounter.h"

//==============================================================================
// Declarations
//==============================================================================
#define LOG_TAG    "FastCVSampleRenderer.cpp"
#define DPRINTF(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define IPRINTF(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define EPRINTF(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

/// Green pixel height width.
static const uint32_t GREEN_PIXEL_HEIGHT_WIDTH  = 5; //3;

//corner point with
static const uint32_t CORNER_PIXEL_HEIGHT_WIDTH  = 7; //3;

static bool init = true;

//------------------------------------------------------------------------------
/// @brief
///    Contains state information of the instance of the application.
//------------------------------------------------------------------------------
struct RendererState
{
   //---------------------------------------------------------------------------
   /// Constructor for State object sets variables to default values.
   //---------------------------------------------------------------------------
   RendererState()
   {
      cameraRenderer = NULL;

      renderBufRGB565 = NULL;
      renderBufSize = 0;
      renderBufWidth = 0;
      renderBufHeight = 0;
      renderBufReady = false;
   }

   /// Pointer to RGB renderer for preview frame.
   CameraRendererRGB565GL2*   cameraRenderer;

   /// Image Buffer to copy image into.
   uint8_t*                   renderBufRGB565;

   /// Display Size (in bytes) of image being rendered
   uint32_t                   renderBufSize;

   /// Width of renderer buffer
   uint32_t                   renderBufWidth;

   /// Height of renderer buffer
   uint32_t                   renderBufHeight;

   /// Flag to protect image buffer data during update from being
   /// rendered.
   bool                       renderBufReady;

   /// Pthread mutex storage.
   pthread_mutex_t            mutex;
};

/// Rendering module's state structure to hold global state for sample app.
static RendererState          state;

//==============================================================================
// Function Declarations
//==============================================================================

//==============================================================================
// Function Definitions
//==============================================================================

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
uint8_t* getRenderBuffer( uint32_t w, uint32_t h )
{
   lockRenderBuffer();

   if(init){
      rpcmem_init();
      init = false;
   }

   // Resize if necessary.
   if( w != state.renderBufWidth || h != state.renderBufHeight )
   {
      if( state.renderBufRGB565 != NULL )
      {
         //fcvMemFree( state.renderBufRGB565 );
         rpcmem_free( (void *)state.renderBufRGB565 );
         state.renderBufRGB565 = NULL;
         state.renderBufSize = 0;
         state.renderBufWidth = 0;
         state.renderBufHeight = 0;
      }
   }

   // Allocate if necessary.
   if( state.renderBufRGB565 == NULL )
   {
      state.renderBufSize = w * h * 2;
      //state.renderBufRGB565 = (uint8_t*) fcvMemAlloc(state.renderBufSize, 16);
      state.renderBufRGB565 = (uint8_t*) rpcmem_alloc(25,RPCMEM_DEFAULT_FLAGS,state.renderBufSize);
      state.renderBufWidth = w;
      state.renderBufHeight = h;

      glEnable(GL_TEXTURE_2D);
      glEnable(GL_BLEND);
      glDisable(GL_DITHER);
      glDisable(GL_DEPTH_TEST);
      glBlendFunc(GL_ONE, GL_SRC_COLOR);

      // Initialize mutex
      pthread_mutexattr_t  attribute;
      pthread_mutexattr_init( &attribute );
      pthread_mutexattr_settype( &attribute, PTHREAD_MUTEX_NORMAL );
      pthread_mutex_init( &state.mutex, &attribute );
   }

   unlockRenderBuffer();

   return state.renderBufRGB565;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
uint32_t getRenderBufferSize()
{
   return state.renderBufSize;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void unlockRenderBuffer()
{
   pthread_mutex_unlock( &(state.mutex) );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void lockRenderBuffer()
{
   int rc = pthread_mutex_lock( &(state.mutex) );
   if( rc != 0 )
   {
      EPRINTF( "Error locking mutex: %d\n", rc );
   }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void drawCorners( uint32_t* corners, uint32_t numCorners, char color, int lineWidth)
{
      uint8_t color_v0, color_v1;

    //RGB565: [0]->[G(3L) B(5)], [1]->[R(5) G(3H)]
      switch(color)
      {
         case 'g':
           //color_v0 = 0;
           //color_v1 = 31;
           color_v0 = 224;
           color_v1 = 7;
           break;

         case 'b':
           color_v0 = 31; //248;
           color_v1 = 0;
           break;

         case 'y':
           color_v0 = 224; //3;
           color_v1 = 255;
           break;

         case 'r':
           color_v0 = 0; //7;
           color_v1 = 248; //224;
           break;

         default:
           color_v0 = 224;
           color_v1 = 7;
           break;

      }


   uint32_t  curcornerx =0;
   uint32_t  curcornery =0;

   // for each of the corner found, insert a green pixel
   for( uint32_t  k=0; k<numCorners; k++ )
   {
      curcornerx = *corners++;
      curcornery = *corners++;

      if ( curcornerx>=state.renderBufWidth || curcornerx+lineWidth>=state.renderBufWidth)
           continue;

      if ( curcornery>=state.renderBufHeight || curcornery+lineWidth>=state.renderBufHeight)
           continue;

      for (uint32_t  i= curcornerx;
           i< lineWidth + curcornerx; i++)
      {
         for (uint32_t j=curcornery;
              j< lineWidth +curcornery; j++)
         {
            //processing on RGB data
            state.renderBufRGB565[ 2* (j*state.renderBufWidth +i)] = color_v0;
            state.renderBufRGB565[ 2* (j*state.renderBufWidth +i) + 1] = color_v1;
         }
      }
   }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void drawBoundingBox( int x,int y,int w,int h, char color, int lineWidth )
{
   uint8_t color_v0, color_v1;
   //RGB565: [0]->[G(3L) B(5)], [1]->[R(5) G(3H)]
   switch(color)
   {
       case 'g':
           //color_v0 = 0;
           //color_v1 = 31;
           color_v0 = 224;
           color_v1 = 7;
           break;

       case 'b':
           color_v0 = 31; //248;
           color_v1 = 0;
           break;

       case 'y':
           color_v0 = 224; //3;
           color_v1 = 255;
           break;

       case 'r':
           color_v0 = 0; //7;
           color_v1 = 248; //224;
           break;

       case 'k':
           color_v0 = 0; //7;
           color_v1 = 0; //224;
           break;

       case 'w':
           color_v0 = 255;
           color_v1 = 255;
           break;

       default:
           color_v0 = 255;
           color_v1 = 255;
           break;

   }


   int i,j;
   //draw the top line
   for ( i = x;i<x+w;i++)
   {
       for ( j=y;j< lineWidth +y; j++)
       {
       //processing on RGB data
            state.renderBufRGB565[ 2* (j*state.renderBufWidth +i)] = color_v0;
            state.renderBufRGB565[ 2* (j*state.renderBufWidth +i) + 1] = color_v1;
       }
   }

   //draw the bottom line
   for ( i = x;i<x+w+lineWidth;i++)
   {
       for ( j=y+h;j< lineWidth +y+h; j++)
       {
       //processing on RGB data
            state.renderBufRGB565[ 2* (j*state.renderBufWidth +i)] = color_v0;
            state.renderBufRGB565[ 2* (j*state.renderBufWidth +i) + 1] = color_v1;
       }
   }


   //draw the left line
   for (  i= x; i< lineWidth + x; i++)
   {
       for ( j=y;j< y+h; j++)
       {
       //processing on RGB data
            state.renderBufRGB565[ 2* (j*state.renderBufWidth +i)] = color_v0;
            state.renderBufRGB565[ 2* (j*state.renderBufWidth +i) + 1] = color_v1;
       }
   }

   //draw the right line
   for (  i= x+w; i< lineWidth + x+w; i++)
   {
       for ( j=y;j< y+h+lineWidth; j++)
       {
       //processing on RGB data
            state.renderBufRGB565[ 2* (j*state.renderBufWidth +i)] = color_v0;
            state.renderBufRGB565[ 2* (j*state.renderBufWidth +i) + 1] = color_v1;
       }
   }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void imagePatchZoom(fcvRectangleInt &srcBbox, fcvRectangleInt &destBbox, float xstep, float ystep)
{

    //check boundary
    if ( (srcBbox.x<=0 || srcBbox.x+srcBbox.width>=state.renderBufWidth || srcBbox.y <= 0 || srcBbox.y+srcBbox.height>=state.renderBufHeight) ||
        (destBbox.x<=0 || destBbox.x+destBbox.width>=state.renderBufWidth || destBbox.y <= 0 || destBbox.y+destBbox.height>=state.renderBufHeight) )
        return;


    int srcsize = srcBbox.width*srcBbox.height;
    int destsize = destBbox.width * destBbox.height;

    //1. to crop srcBbox out, convert into RGB
    uint8_t *srcRGB = (uint8_t *)malloc(srcsize*3*sizeof(uint8_t));
    uint8_t *rgb565_line = state.renderBufRGB565 + 2* (srcBbox.y * state.renderBufWidth + srcBbox.x);
    uint8_t *rgb565;

    uint8_t *srcRGB0, *srcRGB1, *srcRGB2;
    uint8_t *srcRGB_line;
    srcRGB0 = srcRGB;
    srcRGB1 = srcRGB0 + srcsize;
    srcRGB2 = srcRGB1 + srcsize;
    uint8_t c0, c1;
    for (unsigned y=srcBbox.y; y<srcBbox.y+srcBbox.height; y++)
    {
        rgb565 = rgb565_line;
        for (unsigned x=srcBbox.x; x<srcBbox.x+srcBbox.width; x++)
        {
            //RGB565: [0]->[G(3L) B(5)], [1]->[R(5) G(3H)]
            c0 = rgb565[1];
            c1 = rgb565[0];
            *srcRGB0++ = c0 & 248;
            *srcRGB1++ = ((c0 & 7) << 3)+(c1 >> 5);
            *srcRGB2++ = c1 & 31;

            rgb565 += 2;
        }
        rgb565_line += 2*state.renderBufWidth;
    }

    //2. sample srcBBox RGB into destBBox 565
    uint8_t *dest565_line = state.renderBufRGB565 + 2* (destBbox.y * state.renderBufWidth + destBbox.x);
    uint8_t *dest565;
    srcRGB_line= srcRGB;
    uint8_t *srcRGB0_nextrow, *srcRGB1_nextrow,*srcRGB2_nextrow;

    uint8_t rgb0, rgb1, rgb2;
    float wtx, wty, nwtx, nwty;
    wty = 0;
    for (unsigned y=destBbox.y;y<destBbox.y+destBbox.height-2;y++)
    {
        nwty = 1-wty;
        wtx = 0;
        dest565 = dest565_line;
        srcRGB0 = srcRGB_line;
        srcRGB1 = srcRGB0 + srcsize;
        srcRGB2 = srcRGB1 + srcsize;
        srcRGB0_nextrow = srcRGB0 + srcBbox.width;
        srcRGB1_nextrow = srcRGB1 + srcBbox.width;
        srcRGB2_nextrow = srcRGB2 + srcBbox.width;
        for (unsigned x=destBbox.x;x<destBbox.x+destBbox.width-2;x++)
        {
            nwtx = 1-wtx;

            //set rgb values

            rgb0 = (uint8_t) ( ((float)(*srcRGB0)*nwtx + (float)(*(srcRGB0+1))*wtx)*nwty +
                             ((float)(*(srcRGB0_nextrow))*nwtx + (float)(*(srcRGB0_nextrow+1))*wtx)*wty );

            rgb1 = (uint8_t) ( ((float)(*srcRGB1)*nwtx + (float)(*(srcRGB1+1))*wtx)*nwty +
                             ((float)(*(srcRGB1_nextrow))*nwtx + (float)(*(srcRGB1_nextrow+1))*wtx)*wty );

            rgb2 = (uint8_t) ( ((float)(*srcRGB2)*nwtx + (float)(*(srcRGB2+1))*wtx)*nwty +
                             ((float)(*(srcRGB2_nextrow))*nwtx + (float)(*(srcRGB2_nextrow+1))*wtx)*wty );

            //rgb0 = srcRGB0[0];
            //rgb1 = srcRGB1[0];
            //rgb2 = srcRGB2[0];

            //set into dest565
            //RGB565: [0]->[G(3L) B(5)], [1]->[R(5) G(3H)]
            dest565[1] = (rgb0 & 248) + ((rgb1 & 56)>>3);
            dest565[0] =  (rgb1<<5) + (rgb2 & 31);

            //move source in x
            wtx += xstep;
            while(wtx>=1)
            {
                wtx--;
                srcRGB0++;
                srcRGB1++;
                srcRGB2++;
                srcRGB0_nextrow++;
                srcRGB1_nextrow++;
                srcRGB2_nextrow++;
            }

            //move destination in x
            dest565 += 2;
        }
    //move source in y
        wty += ystep;
        while(wty>=1)
        {
            wty--;
            srcRGB_line += srcBbox.width;
        }
        //move destination in y
        dest565_line += state.renderBufWidth * 2;
    }
    //release memory
    free(srcRGB);

    //draw a rectangle around the zoomed patch
    drawBoundingBox(destBbox.x,destBbox.y,destBbox.width,destBbox.height, 'w', 2);

}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
JNIEXPORT void JNICALL
   Java_com_qualcomm_qti_touch2track_FastCVSampleRenderer_cleanup
(
   JNIEnv * /*env*/,
   jobject /*obj*/
)
{
   DPRINTF( "%s\n", __FUNCTION__ );

   if( state.cameraRenderer != NULL )
   {
      state.cameraRenderer->UnInit();
      state.cameraRenderer = NULL;
   }

   if( state.renderBufRGB565 != NULL )
   {
      //fcvMemFree( state.renderBufRGB565 );
      rpcmem_free( (void *)state.renderBufRGB565 );
      state.renderBufRGB565 = NULL;
   }

   return;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
JNIEXPORT void JNICALL
   Java_com_qualcomm_qti_touch2track_FastCVSampleRenderer_surfaceChanged
(
   JNIEnv * /*env*/, jobject /*obj*/,
   jint width,
   jint height
)
{
   DPRINTF( "Viewport changed to %ux%u \n", width, height );

   //Set up viewport. We only need to do this once.
   glViewport(0, 0, width, height);

   return;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_com_qualcomm_qti_touch2track_FastCVSampleRenderer_render
(
   JNIEnv * /*env*/,
   jobject /*obj*/,
   jbyteArray /*img*/,
   jint /*w*/,
   jint /*h*/
)
{
   if( state.cameraRenderer == NULL )
   {
      state.cameraRenderer = new CameraRendererRGB565GL2();

      if (state.cameraRenderer)
      {
         state.cameraRenderer->Init();
      }
      else
      {
         EPRINTF( "Unable to get Camera Render object\n" );
      }
   }

   //if camera frame is ready, display that
   if( state.cameraRenderer && state.renderBufRGB565 )
   {
      lockRenderBuffer();
      state.cameraRenderer->Render
      (
         state.renderBufRGB565,
         state.renderBufWidth,
         state.renderBufHeight
      );
      unlockRenderBuffer();

      //glFlush();
      glFinish();
   }

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
JNIEXPORT jfloat JNICALL Java_com_qualcomm_qti_touch2track_FastCVSampleRenderer_getGPUFPS
(
   JNIEnv*  /*env*/,
   jobject  /*obj*/
)
{
   jfloat rc = 0.f;

   if ( state.cameraRenderer )
   {
      rc = state.cameraRenderer->GetRenderingFPS();
   }
   return rc;
}

